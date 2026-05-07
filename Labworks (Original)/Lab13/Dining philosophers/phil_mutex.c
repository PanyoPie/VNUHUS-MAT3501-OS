/* Dining philosophers using semaphore, breaking Mutex condition:  
   Create an arbiter thread to maintain a queue of chopstick requests and release actions
   Philosophers have to ask for chopsticks and inform the arbiter when releasing chopsticks
   Semaphore arbiter_requests is used to synchorize between philosophers and the arbiter
   test() function checks if 2 chopsticks are available then raises up semaphore S[i]
   The philosopher calls down(&S[i]) to take both chopsticks or otherwise sleep waiting
   Notes: 
		- When a philosopher asking for chopsticks, his ID is added to the end of the queue
		- When a philosopher releases chopsticks, he informs the arbiter to test for other philosophers
		  in the queue
*/
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT(i) (i+N-1)%N
#define RIGHT(i) (i+1)%N
#define R 3
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
  
 
sem_t arbiter_requests; // requests to the arbiter
sem_t mutex;			// mutex is used for the printing purpose only
sem_t S[N];				// S[i] represents whether philosopher i can eat
sem_t queue_lock; 		// lock the queue to avoid race condition

#include "queue.c"		// queue functions 

void * philosopher(void *pNum);
void * arbiter(void *pArg);
void arbiter_request(int);
void arbiter_release(int);
void test(int i);
void think(int);
void eat(int);
void print_philosophers(void);
  
int state[N];
char state_label[N][10];  // for printing purpose only
int phil_num[N];

int main()
{
    int i;
    pthread_t thread_id[N], arbiter_id;
    sem_init(&mutex,0,1);
	sem_init(&queue_lock,0,1);
	sem_init(&arbiter_requests,0,0);
    for(i=0;i<N;i++)
        sem_init(&S[i],0,0);
	pthread_create(&arbiter_id,NULL,arbiter,NULL);
    for(i=0;i<N;i++) {
		phil_num[i]=i;
        pthread_create(&thread_id[i],NULL,philosopher,&phil_num[i]);
    }
    for(i=0;i<N;i++)
        pthread_join(thread_id[i],NULL);
	pthread_join(arbiter_id,NULL);
}

/*
The arbiter is woken up whenever there is a request for chopsticks or release chopsticks
He scans through the queue to check which philosophers can eat. If not, add back to the queue.
*/
void *arbiter(void *pArg) {
	int id, i, size;
	char ch;
	while (1) {
		down(&arbiter_requests);	// get a request or sleep
		
		for (i=0; i<get_queue_size(); i++) { // scan through the queue
			if (!get_queue_request(&id)) break; // queue empty
			test(id); 
			if (state[id]!=EATING) // test() fails
				add_queue_request(id); // put back to the queue
		}
	}
}

void *philosopher(void *pNum) {
	int i=*(int*)pNum;
	strcpy(state_label[i],"( T )");
	while(1)
    {
        think(i); 
        arbiter_request(i);	// request the arbiter for eating permission
        eat(i);
        arbiter_release(i);	// inform the arbiter to release the chopsticks
    }
}

/*
Add the request for chopsticks to the arbiter's queue
*/
void arbiter_request(int i) {
	down(&mutex);
	state[i] = HUNGRY;
	strcpy(state_label[i],"( H )");
	printf("Philosopher %d is asking for chopsticks\n",i+1);
	print_philosophers();
	up(&mutex);
	add_queue_request(i);
	up(&arbiter_requests);
	
    down(&S[i]);
    sleep(1);
}

/*
Inform the arbiter when chopsticks are released.
*/
void arbiter_release(int i) {
	down(&mutex);
	state[i] = THINKING;
	strcpy(state_label[i],"( T )");
	printf("Philosopher %d puts chopsticks down\n",i+1);
	print_philosophers();
	up(&mutex);
	up(&arbiter_requests);
	
    sleep(1);
}

/*
Test if philosopher i can eat, if yes, raise up semaphore S[i]
*/
void test(int i) {
	char ch;
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING)
    {
		down(&mutex);
        state[i] = EATING;
		strcpy(state_label[i],"(|E|)");
        up(&S[i]);
		
		printf("Philosopher %d is eating\n", i+1);
		print_philosophers();
		up(&mutex);
    }
}
  
void think(int i) {
	//printf("Philosopher %d is thinking\n", i+1);
	//print_philosophers();
	sleep(rand()% R);
}

void eat(int i) {
	//printf("Philosopher %d is eating\n", i+1);
	//print_philosophers();
	sleep(rand()% R);
}

void print_philosophers(void) {
	int i;
	char ch;
	//down(&mutex);
	for (i=0; i<N; i++) {
		if (state_label[i][3]=='|' || state_label[RIGHT(i)][1]=='|') ch = ' ';
		else ch = '|';
		printf("%s %c ", state_label[i],ch);
	}
	printf("\n");
	//up(&mutex);
}