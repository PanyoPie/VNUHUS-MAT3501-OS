//Philosopher with deadlocks

#include<stdio.h>    
#include <unistd.h>
#include <stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define N 5
#define R 3
#define TRUE 1
#define LEFT(i)	i
#define RIGHT(i) (i+1)%N
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
#define DELAY	sleep(rand() % R);

sem_t chopstick[N]; // each chopstick is represented as a semaphore

void* philosopher(void *pNo) { // pNo points to a philosopher ID
	int i= ...;  // i is the ID number of the philosopher
	
	while (TRUE) {
		// Think
		...
		
		// Hungry
		...
		
		// Take left chopstick
		...
		// Take right chopstick
		...
		
		// Eat
		...
		
		// Drop left chopstick
		...
		// Drop right chopstick
		...
	}
}


int main() {
	pthread_t tid[N];
	int philosopherID[N], i;

	// Initiate semaphores
	for (i=0; i<N; i++) sem_init(&chopstick[i],0,1);
	
	// Create philosopher threads
	for (i=0; i<N; i++) {
		philosopherID[i]=i;
		pthread_create(&tid[i],NULL,philosopher,&philosopherID[i]);
	}
	
	// Wait till all philosophers terminate
	for (i=0; i<N; i++) 
		pthread_join(tid[i],NULL);
}

