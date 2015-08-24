#include "defs.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

int fd;			//file descriptor
int sem_own = 0;	//ownership of gender semaphore
semaphore_str *sem_ptr;	//semaphore pointer

//enter the bathroom when same gender is in////////////////////////////////////
int acq_same(my_sex)
{
	sem_ptr->same_wait = sem_ptr->same_wait + 1;	//you have to increment same wait even if there is place in the bathroom for you
	if (sem_ptr->curr_gender != my_sex)	//you might have be timed out and everyone else of your gender left the bathroom and the curr_gender has changed- you will have no way of knowing
	{
		sem_ptr->same_wait = sem_ptr->same_wait - 1;	//roll back if curr_gender has changed while you were swapped out. this is a complete rollback, since you never grabbed a semaphore or prevented any other process from doing so
		return (0);	//0-let the calling function know that you have not managed to enter the bathroom
	}
	if(sem_wait(&(sem_ptr->bathroom)) != 0)	//acquire semaphore. if failed- something is wrong, exit
	{
		perror("ACQ_SAME ERROR: ");
		exit (-1);
	}
	sem_ptr->space_left = sem_ptr->space_left - 1;	//have successfully entered bathroom- reduce space_left
	sem_ptr->same_wait = sem_ptr->same_wait - 1;	//since you have entered and not waiting, decrement same_wait
	return (1);	//1- let calling function know that you have successfully entered the bathroom
}
//enter the bathroom when same sex is in - end/////////////////////////////////

//try enter the bathroom - start///////////////////////////////////////////////
//Three conditions: Bathroom is empty, Bathroom has processes of same sex, Bathroom has processes of opposite sex////
int bathroomEnter(int my_sex)
{
	int success = 0;	//flag- enter successful: 0-no, 1- yes?
	sem_own = 0;	//flag- do I own the gender semaphore: 0-no, 1- yes?
	if (sem_ptr->curr_gender == 2)	//bathroom is empty- anyone can enter as long as they manage to grab the gender 
	{
		#ifdef enter_check
		printf("empty bathroom\n");
		#endif		
		if (sem_trywait(&(sem_ptr->gender)) == 0)	//try to grab the gender semaphore
		{
			#ifdef enter_check
			printf("acquired gender semaphore\n");	//gender semaphore success
			#endif
			sem_ptr->curr_gender = my_sex;	//change bathroom's curr_gender to my_sex
			success = acq_same(my_sex);	//try entering the bathroom
			//sem_own = 1; //not reqd. only way control can be here is if the process owns gender flag
			if (success != 1)	//failure grabbing bathroom semaphore- something is wrong- exit
			{
				printf("ERROR in acquiring EMPTY bathroom\n");
				exit (-1);	//here we don't have to check for the gender being changed, since this process still holds the gender semaphore
			}
// 			if (sem_post(&(sem_ptr->gender)) != 0)	//drop the gender semaphore. now since bathroom's curr_gender is not neutral(2) no one will try to grab the gender semaphore till curr_gender returns to neutral
// 			{
// 				perror("SEM_POST ERROR: ");
// 				exit (-1);
// 			}
			#ifdef enter_check
				printf("I'm in the bathroom- curr_gender: %d, space_left: %d\n", sem_ptr->curr_gender, sem_ptr->space_left);
			#endif
			if (success == 1)
			return (1);	//notify main() that enter was successfull
		}
		else
		{
			return (0);	//else notify main() that process couldn't successfully enter- main() will send this process again
		}
	}
	else if (sem_ptr->curr_gender == my_sex)	//same gender in bathroom
	{
		success = acq_same(my_sex);	//just go ahead and try to grab the bathroom semaphore to enter
		if (success == 1)
		{
			return (1);	//if success, notify main() that process has entered
		}
		else if (success == 0)	//case of time out and everybody else exiting and curr_gender being changed
		{
			return (0);	//else notify main() that process has not entered
		}
		else	//else something wrong- exit
		{
			printf ("ERROR while trying to get into bathroom with similar occupants\n");
			exit (-1);
		}
	}
	else if (sem_ptr->curr_gender != my_sex)	//different gender
	{
		success = 0;	//flag- have i managed to enter bathroom?
		sem_ptr->opp_wait = sem_ptr->opp_wait + 1;	//increment counter for opposite sex wait
		while (sem_ptr->curr_gender != my_sex)	//keep trying to grab gender semaphore- need while loop because sem_trywait() is used and it is non-blocking
		{
			if (sem_ptr->curr_gender == 2)	//if gender has turned neutral
			{
				if (sem_trywait(&(sem_ptr->gender)) == 0)	//grab gender semaphore
				{
					sem_ptr->curr_gender = my_sex;	//success- turn curr_gender to my_sex
					sem_own = 1;	//I own the gender semaphore
					sem_ptr->opp_wait = sem_ptr->opp_wait - 1;	//decrement opposite wait counter, since now bathroom gender and my_sex are same
					success = acq_same(my_sex);	//acquire bathroom semaphore
					if (success == 1)
					{
						return (1);	//notify main() of success
					}
					else if (success == 0)	//case of time out and everybody else exiting and curr_gender being changed
					{
						return (0);	//notify main() that not able to enter- main() will try to make this `process enter again
					}
					else
					{
						printf ("ERROR while trying to get into bathroom with similar occupants\n");
						exit (-1);	//exit if there is some other failure
					}
				}
				else
				{
					sem_ptr->opp_wait = sem_ptr->opp_wait - 1;	//someone else has grabbed gender semaphore- i will try to enter the bathroom again
					return (0);	//I have not been able to enter- let main() know- main() will ask me to try to enter again
				}	//end of else of if sem_trywait-gender
			}	//end of if curr_gender == 2
		}	//end of while
		sem_ptr->opp_wait = sem_ptr->opp_wait - 1;	//I will go through the process again
		return (0);	//I have not been able to enter- let main() know- main() will ask me to try to enter again
	}	//end of different gender
}	//end of bathroomEnter function
//try bathroomEnter bathroom -end//////////////////////////////////////////////////////

//leave bathroom///////////////////////////////////////////////////////////////
bathroomExit()
{
	if (sem_post(&(sem_ptr->bathroom)) != 0)	//drop the bathroom semaphore
	{
		perror("SEM_POST ERROR: ");
		exit (-1);
	}
	sem_ptr->space_left = sem_ptr->space_left + 1;	//increment space left in the bathroom
	if ((sem_ptr->same_wait == 0) && (sem_ptr->space_left == sem_ptr->space))	//if you are last person in bathroom and counter for same gender processes waiting is zero, set the bathroom gender to neutral
	{
		if (sem_post(&(sem_ptr->gender)) != 0)	//drop the gender semaphore
		{
			perror("SEM_POST ERROR: ");
			exit (-1);
		}
		sem_ptr->curr_gender = 2;
	}
}
//leave bathroom - end/////////////////////////////////////////////////////////
	
	
			

int main(int argc, char *argv[])
{
	int my_sex, time;
	int success = 0;
	
	//Obtain inputs start/////////////////////////////////////////////////////////
	if(argc < 3)	//less than required inputs- error message and exit
	{
		invalid_ip();
	}
	else if((strcmp(argv[1], "w") == 0) || (strcmp(argv[1], "W") == 0))
		my_sex = 0;	//woman
	else if((strcmp(argv[1], "m") == 0) || (strcmp(argv[1], "M") == 0))
		my_sex = 1; //man
	else
	{
		invalid_ip();	//invalid, exit
	}

	time = atoi(argv[2]);	//get time for which bathroom will be used
	if (time < 0)
	{
		invalid_ip();	//invalid time
	}
	
	#ifdef input_check
	printf("User gender: %d, Use time: %d\n", my_sex, time);
	#endif
	
	//Obtain inputs end///////////////////////////////////////////////////////////
	
	//Open common file and map start//////////////////////////////////////////////
	fd = open("room.txt", O_RDWR ,0664);	//open file
	if (fd < 0)
	{
		perror("FILE: "); 
		exit (-1);
	}
	sem_ptr = (semaphore_str*)mmap(0, sizeof(semaphore_str), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	//map semaphore
	if((caddr_t)sem_ptr == (caddr_t)-1)
	{
		perror("MMAP FAILED: ");
		exit(-1);
	}
	close( fd );
	//Mapping to file end/////////////////////////////////////////////////////////
	
	//Enter bathroom start////////////////////////////////////////////////////////
	while (success == 0)	//have the process try to bathroomEnter till it is successfull
	{
		success = bathroomEnter(my_sex);
	}
// 	if (success == 1 && sem_own == 1)	//if the process captured the gender semaphore, it should drop the gender semaphore after successfully entering
// 	{
// 		if (sem_post(&(sem_ptr->gender)) != 0)
// 		{
// 			perror("SEM_POST ERROR: ");
// 			exit (-1);
// 		}
// 	}
	#ifdef display_check
	printf("my_sex: %d\t", my_sex);
	#endif
	printf("Using bathroom\n");
	//Enter bathroom end//////////////////////////////////////////////////////////
	
	//Stay in bathroom////////////////////////////////////////////////////////////
	sleep (time);
	//stay in bathroom - end//////////////////////////////////////////////////////
	
	//Leave bathroom//////////////////////////////////////////////////////////////
	bathroomExit();
	//Leave bathroom - end////////////////////////////////////////////////////////
	
	return (1);
}

