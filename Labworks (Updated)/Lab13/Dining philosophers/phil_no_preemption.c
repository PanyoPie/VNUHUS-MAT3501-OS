/* Dining philosophers using semaphore 
   Breaking No Preemption condition (simple version): 
   - give priority to the right philosopher 
   - a philosopher with higher priority can preempt the chopstick held by the left phisolopher
   - when a philosopher's chopsticks are taken, his state is changed to PREEMPTED
   In this simple version
   - when a eating philosopher is preempted, he will stop eating and proceed to putting the chopsticks 
     down, then get back to thinking
*/
 
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
#define PREEMPTED 3
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
    down(&mutex);
    state[i] = HUNGRY;
	strcpy(state_label[i],"( H )");
    printf("Philosopher %d is Hungry\n",i+1);
	print_philosophers();
	test(i);	// if 2 chopsticks available, raise up S[i]
    up(&mutex);
    
	down(&S[i]);
}
  
// Only need to check if the right chopstick is available
// Left chopstick can be preempted using higher priority
void test(int i) {
	int v;
	if (state[i]==HUNGRY && (state[RIGHT(i)]==THINKING || state[RIGHT(i)]==HUNGRY)) {	
		// right chopstick is available and
		// if the left philosopher with lower priority is eating, force him to drop the chopsticks and wait
		if (state[LEFT(i)]==THINKING || state[LEFT(i)]==HUNGRY) {
			printf("Philosopher %d takes chopsticks %d and %d\n",i+1,i+1,RIGHT(i)+1);
			state[i]=EATING;
			strcpy(state_label[i],"(|E|)");
			print_philosophers();
			up(&S[i]); // 2 chopsticks are ready
		}
		else { // left chopstick is already taken
			state[LEFT(i)]=PREEMPTED;		// force the left philosopher drop down the 2 chopsticks
			printf("Philosopher %d is asked to give chopstick to philosopher %d\n", LEFT(i)+1, i+1);
			strcpy(state_label[LEFT(i)],"(|P|)");
			print_philosophers();
		}
    }
}
  
void put_chopsticks(int i){
    down(&mutex);
    state[i] = THINKING;
    printf("Philosopher %d puts chopsticks %d and %d down\n",i+1,i+1,RIGHT(i)+1);
	strcpy(state_label[i],"( T )");
	print_philosophers();
    test(RIGHT(i));
	test(LEFT(i));
    up(&mutex);
}

void think(int i) {
	//printf("Philosopher %d is thinking\n", i+1);
	sleep(rand()% R);
}

void eat(int i) {
	int j=0, time=30;	// 3 seconds to eat
	for (j=0; j<time; j++) { // eat as long as 3 seconds
		if (state[i]==EATING) usleep(100000); // 0.1s
		else break; // until someone preempts the chopsticks
	}
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