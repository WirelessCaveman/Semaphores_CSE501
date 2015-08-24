#include "defs.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>


semaphore_str sem_var;
semaphore_str *sem_ptr;
int fd;	//file descriptor


void leave(int sig);

int main (int argc, char *argv[])
{
	int size, interval	= 0;	//size of bathroom and interval of printing
	
	//Read in arguments start//////////////////////////////////////////////////
	if (argc < 2)
	{
		invalid_ip();	//prints out expected input formats and exits
	}
	
	if(argc > 2)	//the interval argument is present
	{
		interval = atoi(argv[2]);
		if (interval < 1)
		{
			invalid_ip();
		}
	}
	else
		interval = 1;	//default interval argument
	
	size = atoi(argv[1]);	//size of bathroom
	if (size < 1)
	{
		invalid_ip();
	}
	//Read arguments end///////////////////////////////////////////////////////
	
	//Initialize the variables of the semaphore data structure variable sem_var///////
	sem_var.space = size;
	sem_var.space_left = size;
	sem_var.curr_gender = 2;	//women- 0, men- 1, none- 2
	sem_var.same_wait = 0;	//a counter of number of processes of the same sex that are waiting
	sem_var.opp_wait = 0;	//a counter of number of processes of different sex that are waiting
	//Initialize end//////////////////////////////////////////////////////////////
	
	//Create semaphores for bathroom and waiting//////////////////////////////////
	if (0 != sem_init (&sem_var.bathroom, PSHARED, size))	//semaphore for bathroom
	{
		perror("BathRoom semaphore ERROR:");
	}
	if (0 != sem_init (&sem_var.gender, PSHARED, 1))	//semaphore for gender- only one person holds this
	{
		perror("Gender semaphore ERROR:");
	}
	//Create semaphores end///////////////////////////////////////////////////////
	
	//write the semaphore variable in the file and map it to a local object///////
	if(access("room.txt",F_OK) == 0)	//if the shared file exists from a past run, delete it
	{
	  if(unlink("room.txt") != 0) 
		{
			perror("Delete failed:");
			exit(-1);
		}
	}
	fd = open ("room.txt", O_RDWR|O_CREAT, 0664);	//create shared file
	if (fd < 0)
	{
		perror("FILE ERROR: "); 
		exit (-1);
	}
	write (fd, &sem_var, sizeof (semaphore_str));	//write the semaphore data structure into the file
	sem_ptr = (semaphore_str*)mmap(0,sizeof(semaphore_str), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);	//map the local semaphore
	if (sem_ptr == MAP_FAILED)
	{
		printf("MMAP failure\n");
	}
	if((caddr_t)sem_ptr == (caddr_t)-1)	//caddr_t is a char * 
	{
		perror("MMAP ERROR: ");
		exit(-1);
	}
	fsync(fd); //make sure write to file is completed
//	sleep(1);	//simply wait to print in a more readable speed
	close (fd);
	//end of writing to the file//////////////////////////////////////////////////
	
	
	signal(SIGINT,leave);	//wait for ctrl+c signal
	
	
	//display status- start////////////////////////////////////////////////////////
	while(1)
	{
		sleep(interval);
		#ifdef display_check
		printf ("current gender: %d, space_left: %d\n", sem_ptr->curr_gender, sem_ptr->space_left);
		#endif
   	switch(sem_ptr->curr_gender)
		{
			case 2:
				printf("Women %d, Men %d\n", (sem_ptr->space - sem_ptr->space_left), (sem_ptr->space - sem_ptr->space_left));
				break;
			case 0:
				printf("Women %d, Men %d\n", (sem_ptr->space - sem_ptr->space_left), 0 );
				break;
			case 1:
				printf("Women %d, Men %d \n", 0, (sem_ptr->space - sem_ptr->space_left));
				break;
			default:
				exit (-1);
		}
  }
  //end of display/////////////////////////////////////////////////////////////
  
  return (0);	//forced exit
  
}

void leave(int sig) //delete the file upon receiving a ctrl+c
{
	close(fd);
	if(unlink("room.txt") != 0) 
	{
		perror("Delete failed: ");
		exit(-1);
	}
	exit(sig);
}


