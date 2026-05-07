// Dining philosophers using semaphore
// Breaking Hold and Wait condition: take both chopsticks at once or wait
// test() function checks if 2 chopsticks are available then raises up semaphore s[i]
// The philosopher calls down(&s[i]) to take both chopsticks or otherwise sleep waiting
 
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
#define LEFT(i) (i+N-1)%N
#define RIGHT(i) (i+1)%N
#define R 3
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
  
sem_t mutex;
sem_t S[N];
  
void * philosopher(void *pNum);
void take_chopsticks(int);
void put_chopsticks(int);
void test(int);
void think(int);
void eat(int);
void print_philosophers(void);
  
int state[N];
char state_label[N][10];  // for printing purpose only
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
	int i=*(int*)pNum;
	strcpy(state_label[i],"( T )");
	while(1) {
        think(i);
        take_chopsticks(i);
        eat(i);
        put_chopsticks(i);
    }
}
  
void take_chopsticks(int i)
{
    down(&mutex);
    state[i] = HUNGRY;
	strcpy(state_label[i],"( H )");
    printf("Philosopher %d is Hungry\n",i+1);
	print_philosophers();
    test(i);
    up(&mutex);
    down(&S[i]);
    sleep(1);
}
  
void test(int i)
{
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING)
    {
        state[i] = EATING;
        sleep(2);
        printf("Philosopher %d takes chopsticks %d and %d\n",i+1,i+1,RIGHT(i)+1);
		strcpy(state_label[i],"(|E|)");
		print_philosophers();
        up(&S[i]);
    }
}
  
void put_chopsticks(int i)
{
    down(&mutex);
    state[i] = THINKING;
    printf("Philosopher %d puts chopsticks %d and %d down\n",i+1,i+1,RIGHT(i)+1);
	strcpy(state_label[i],"( T )");
	print_philosophers();
    test(LEFT(i));
    test(RIGHT(i));
    up(&mutex);
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
	
	for (i=0; i<N; i++) {
		if (state_label[i][3]=='|' || state_label[RIGHT(i)][1]=='|') ch = ' ';
		else ch = '|';
		printf("%s %c ", state_label[i],ch);
	}
	printf("\n");
	
}