//prehistoric cave visitors

#include<stdio.h>             
#include<pthread.h>
#include<semaphore.h>

#define N 5		// Max visitors in the cave
#define MAXTHREADS 20
#define TRUE 1
#define FALSE 0
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
#define DELAY(R)	sleep(rand() % R);

sem_t visitors, mutex;
int noOfVisitors;

void* caveVisitor(void* id) {
	// Wait until one's turn
	...		// get a ticket, sleep otherwise
	...		// increase number of visitors in the cave
	printf("There are %d visitors\n", noOfVisitors);

	// Entering the cave through a 1 man passage
	...		// enter critical section
	printf("Visitor %d is entering... ", *((int*)id));
	DELAY(3);	// it takes a while
	...		// leave critical section
	printf("now in the cave.\n");

	// Contemplating
	printf("Visitor %d is contemplating... \n", *((int*)id));
	DELAY(3);	// it takes a while
	printf("Visitor %d is finished.\n", *((int*)id));

	// Getting out through the 1 man passage
	...		// enter critical section
	printf("Visitor %d is getting out... ", *((int*)id));
	DELAY(3);	// it takes a while
	printf("Visitor %d is out of the cave.\n",*((int*)id));
	...		// leave critical section
	...		// decrease number of visitors in the cave
}

int main() {
	pthread_t tid[MAXTHREADS];
	int visitorID[MAXTHREADS];
	int i;

	sem_init(&mutex,0,1);
	sem_init(&visitors,0,N);
	
	for (i=0; i<MAXTHREADS; i++) {
		visitorID[i]=i+1;
		...		// create caveVisitor thread
		DELAY(3);
	}
	for (i=0; i<MAXTHREADS; i++) {
		pthread_join(tid[i],NULL);
	}
}

