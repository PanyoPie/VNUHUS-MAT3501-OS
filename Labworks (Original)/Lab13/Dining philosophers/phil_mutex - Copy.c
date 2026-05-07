/* Dining philosophers using semaphore, breaking Mutex condition:  
   Create an arbiter thread to maintain a queue of chopstick requests and release actions
   Philosophers have to ask for chopsticks and inform the arbiter when releasing chopsticks
   Semaphore arbiter_signal is used to synchorize between philosophers and the arbiter
   test() function checks if 2 chopsticks are available then raises up semaphore S[i]
   The philosopher calls down(&S[i]) to take both chopsticks or otherwise sleep waiting
   Notes: 
		- When a philosopher asking for chopsticks, his ID is added to the end of the queue
		- When a philosopher releases chopsticks, the negative value of his ID is inserted 
		  to the beginning of the queue (not quite a FIFO queue).
*/
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
  
 
sem_t mutex, queue_lock, arbiter_signal;
sem_t S[N];		// S[i] represents whether philosopher i can eat

#include "queue.c"

void * philosopher(void *pNum);
void * arbiter(void *pArg);
void arbiter_request(int);
void arbiter_release(int);
void test(int i);
void think(int);
void eat(int);
  
int state[N];
int phil_num[N];

int main()
{
    int i;
    pthread_t thread_id[N], arbiter_id;
    sem_init(&mutex,0,1);
	sem_init(&queue_lock,0,1);
	sem_init(&arbiter_signal,0,0);
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
		down(&arbiter_signal);
		
		for (i=0; i<get_queue_size(); i++) { // scan through the queue
			if (!get_queue_request(&id)) break; // queue empty
			if (id>0) {	// request for chopsticks
				test((id-1)); // phil id is from 0..4
				if (state[id-1]!=EATING) // test() fails
					add_queue_request(id);
			} else if (id<0) {	// release chopsticks
				id = -id;
				test(LEFT(id-1));
				test(RIGHT(id-1));
			}
		}
	}
}

void *philosopher(void *pNum) {
	int i=*(int*)pNum;
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
Note that the philosopher ID added to the queue is from 1..5 (not from 0..4)
*/
void arbiter_request(int i) {
	state[i] = HUNGRY;
	add_queue_request(i+1);	// in the queue philosopher id is from 1..5
	up(&arbiter_signal);
	//printf("Philosopher %d is asking for chopsticks\n",i+1);

    down(&S[i]);
    sleep(1);
}

/*
Insert the action of releasing chopsticks to the beginning of arbiter's queue (not quite FIFO).
The value inserted to the queue is the negative value of philosopher ID (from -1..-5)
*/
void arbiter_release(int i) {
    insert_queue_request(-(i+1));
	state[i] = THINKING;
	//printf("Philosopher %d puts chopsticks down\n",i+1);
	up(&arbiter_signal);
	
    sleep(1);
}

/*
Test if philosopher i can eat, if yes, raise up semaphore S[i]
*/
void test(int i) {
	char ch;
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING)
    {
        state[i] = EATING;
        up(&S[i]);
		
		// print out philosophers' state
		for (i=0; i<N; i++) {
			if (state[i]==THINKING) ch='T';
			else if (state[i]==HUNGRY) ch='H';
			else ch='E';
			printf(" %c", ch);
			
		}
		printf("\n");
    }
}
  
void think(int i) {
	//printf("Philosopher %d is thinking\n", i+1);
	sleep(rand()% R);
}

void eat(int i) {
	//printf("Philosopher %d is eating\n", i+1);
	sleep(rand()% R);
}