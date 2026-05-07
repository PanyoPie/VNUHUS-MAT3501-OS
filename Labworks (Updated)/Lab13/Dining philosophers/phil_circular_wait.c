// Dining philosophers using semaphore
// Breaking Circular Wait condition: always take the chopstick with smallest index first
// Chopsticks are represented as an array of semaphore
 
#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<semaphore.h>
#include<pthread.h>
  
#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define RIGHT(j) (j+1)%N
#define FIRST(i) (i<(i+1)%N?i:(i+1)%N)
#define SECOND(i) (i<(i+1)%N?(i+1)%N:i)
#define R 3
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
  
sem_t chopstick[N]; // each chopstick is represented as a binary semaphore
sem_t mutex;		// for printing purpose only
  
void * philosopher(void *pNum);
void take_chopsticks(int);
void put_chopsticks(int);
void think(int);
void eat(int);
void print_philosophers(void);
  
int state[N];			  // not needed in this version
char state_label[N][10];  // for printing purpose only
int phil_num[N];
  
int main()
{
    int i;
    pthread_t thread_id[N];
    
	sem_init(&mutex,0,1);
    for(i=0;i<N;i++)
        sem_init(&chopstick[i],0,1);
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
	int i=*(int*)pNum;
	strcpy(state_label[i],"( T )");
	while(1) {
        think(i);
		take_chopsticks(i);
        eat(i);
		put_chopsticks(i);
    }
}
  
void take_chopsticks(int i) {
	state[i] = HUNGRY;
	down(&chopstick[FIRST(i)]);		// take 1st chopstick with smaller index
	printf("Philosopher %d takes chopstick #%d\n", i+1, FIRST(i)+1);
	strcpy(state_label[i],"(|H )");
	print_philosophers();
	if (i!=4) strcpy(state_label[i],"(|H )"); else strcpy(state_label[i],"( H|)");
	down(&chopstick[SECOND(i)]);	// take 2nd chopstick with larger index
	printf("Philosopher %d takes chopstick #%d\n", i+1, SECOND(i)+1);
	strcpy(state_label[i],"(|E|)");
	state[i] = EATING;
	print_philosophers();
}

void put_chopsticks(int i) {
	up(&chopstick[FIRST(i)]);		// put down 1st chopstick with smaller index
	printf("Philosopher %d drops chopstick #%d\n", i+1, FIRST(i)+1);
	up(&chopstick[SECOND(i)]);	// put down 2nd chopstick with larger index
	printf("Philosopher %d drops chopstick #%d\n", i+1, SECOND(i)+1);
	strcpy(state_label[i],"( T )");
	state[i] = THINKING;
	print_philosophers();
}

void think(int i) {
	sleep(rand()% R);
}

void eat(int i) {
	sleep(rand()% R);
}

void print_philosophers(void) {
	int i;
	char ch;
	down(&mutex);
	for (i=0; i<N; i++) {
		if (state_label[i][3]=='|' || state_label[RIGHT(i)][1]=='|') ch = ' ';
		else ch = '|';
		printf("%s %c ", state_label[i],ch);
	}
	printf("\n");
	up(&mutex);
}