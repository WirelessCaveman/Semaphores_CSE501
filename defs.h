#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

typedef struct semaphore_str
{
	sem_t bathroom;	//bathroom semaphore
	sem_t gender;	//current bathroom gender semaphore
	int space;	//total space in bathroom
	int space_left;	//space left in bathroom
	int curr_gender;	//current gender flag
	int same_wait;	//counter for processes of same gender as bathroom waiting
	int opp_wait;	//counter for processes of opposite gender to bathroom waiting
}semaphore_str;


#define PSHARED 1							/* Shared Semaphore */

void invalid_ip (void)	//display error message and exit in case of invalid inputs
{
	printf("Bad inputs\n");
	printf("Input format:\n");
	printf("\tbathroom <size> [<interval>] -  size & interval are +ve non zero integers\n");
	printf("\tenter <w|m|W|M> <time> -  time is a +ve integer\n");
	exit (-1);
}

//#define input_check
//#define enter_check
// #define display_check
