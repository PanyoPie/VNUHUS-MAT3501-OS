// Dining philosophers using semaphore
 
#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<semaphore.h>
#include<pthread.h>
  
#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT(i) (i+N-1)%N
#define RIGHT(i) (i+1)%N
#define R 3
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
#define DELAY	sleep(rand() % R);
  
sem_t mutex;
sem_t S[N];
  
void * philosopher(void *pNum);
void take_chopsticks(int);
void put_chopsticks(int);
void test(int);
void think(int);
void eat(int);
  
int state[N];
int phil_num[N];
  
int main()
{
    int i;
    pthread_t thread_id[N];
    sem_init(&mutex,0,1);
    for(i=0;i<N;i++)
        sem_init(&S[i],0,0);
    for(i=0;i<N;i++)
    {
		phil_num[i]=i;
        pthread_create(&thread_id[i],NULL,philosopher,&phil_num[i]);
    }
    for(i=0;i<N;i++)
        pthread_join(thread_id[i],NULL);
}
  
void *philosopher(void *pNum)
{
	int i=...;
	while(1)
    {
        // think
		...
		// take chopsticks
		...
		// eat
		...
		// put chopsticks
		...
    }
}
  
void take_chopsticks(int i)
{
    // change state to HUNGRY
	// check if 2 chopsticks are available
	// get the 2 chopsticks
	...
	
    DELAY;
}
  
void test(int i)
{
    // check if 2 chopsticks are available and philosopher i is hungry
	//     then change state to EATING, set semaphore S[i] up
	...
}
  
void put_chopsticks(int i)
{
    // change state back to THINKING
	// wake up the left philosopher
	// wake up the left philosopher
	...
}

void think(int i) {
	printf("Philosopher %d is thinking\n", i+1);
	sleep(rand()% R);
}

void eat(int i) {
	printf("Philosopher %d is eating\n", i+1);
	sleep(rand()% R);
}