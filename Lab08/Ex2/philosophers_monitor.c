// Dining philosophers using monitor
 
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
#define DELAY(R)    sleep(rand() % R);
  
void * philosopher(void *pNum);
void take_chopsticks(int);
void put_chopsticks(int);
void test(int);
void think(int);
void eat(int);

// uses this macro to initialize default value to static mutex  
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t self[N];
int state[N];
int phil_num[N];
  
int main()
{
    int i;
    pthread_t thread_id[N];
    
    for(i=0;i<N;i++) {
        state[i]=THINKING;
		phil_num[i]=i;
        pthread_create(&thread_id[i],NULL,philosopher,&phil_num[i]);
    }

    for(i=0;i<N;i++)
        pthread_join(thread_id[i],NULL);
}
  
void *philosopher(void *pNum)
{
	int i=*(int*)pNum;
	while(1)
    {
        think(i);
        take_chopsticks(i);
        eat(i);
        put_chopsticks(i);
    }
}
  
void take_chopsticks(int i)
{
    // lock mutex
    pthread_mutex_lock(&mutex);
    
    state[i] = HUNGRY;
    printf("Philosopher %d is Hungry\n",i+1);
    
    test(i);    // test if 2 chopsticks are available

    while (state[i] != EATING) // wait if chopsticks are not available
        // wait for condition variable self[i]
        pthread_cond_wait(&self[i], &mutex);

    // unlock mutex
    pthread_mutex_unlock(&mutex);
}
  
void test(int i)
{
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING)
    {
        state[i] = EATING;
        printf("Philosopher %d takes chopsticks %d and %d\n",i+1,LEFT(i)+1,RIGHT(i)+1);
        
        // wake up neighbour philosphers when dropping chopsticks
        // this has no effect when taking chopsticks
        pthread_cond_signal(&self[i]);
    }
}
  
void put_chopsticks(int i)
{
    // lock mutex
    pthread_mutex_lock(&mutex);

    state[i] = THINKING;
    printf("Philosopher %d putting chopsticks %d and %d down\n",i+1,LEFT(i)+1,RIGHT(i)+1);
    
    test(LEFT(i));
    test(RIGHT(i));

    // unlock mutex
    pthread_mutex_unlock(&mutex);
}

void think(int i) {
	printf("Philosopher %d is thinking\n", i+1);
	DELAY(3);
}

void eat(int i) {
	printf("Philosopher %d is eating\n", i+1);
	DELAY(3);
}