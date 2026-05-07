/* Dining philosophers using semaphore
   Breaking No Preemption condition (more advanced version): 
   - give priority to the right philosopher 
   - a philosopher with higher priority can preempt the chopstick of the left phisolopher
   - when a philosopher's chopstick is taken, his state is changed to PREEMPTED
   In this more advanced version
   - when a eating philosopher is preempted, he will also put the other chopstick down and 
     suspend his eating, i.e down(&preempted)
   - when the right philosopher returns the chopstick, he will test to wake up the preempted
     philosopher to resume eating, i.e up(&preempted), if both chopsticks are available
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
  
sem_t mutex;		// guard critical section
sem_t preempted;	// preempted chopstick 
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
    sem_init(&mutex,0,1);		// allow 1 thread to enter critical section
	sem_init(&preempted,0,0);	// no preempted chopstick
    for(i=0;i<N;i++)
        sem_init(&S[i],0,0);	// 2 nearby chopsticks are not ready to take
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
		// ... Note that there is a gap between eating and putting chopsticks down
		// Even done with the eating, a philosopher can still be preempted before 
		// puting the chopsticks down. Need to print the right message in put_chopsticks().
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
	if ((state[i]==HUNGRY || state[i]==PREEMPTED) && 
		(state[RIGHT(i)]!=EATING)) {	// right chopstick is available
		// philosopher i can always take the left chopstick because
		// either left chopstick is available or it can be preempted
		if (state[i]==HUNGRY) up(&S[i]); // 2 chopsticks are ready to take
		else up(&preempted);			 // resume the preempted philosopher
		state[i]=EATING;
		if (state[LEFT(i)]==EATING) { // left chopstick is already taken => preempt it
			state[LEFT(i)]=PREEMPTED;		// force the left philosopher drop down the 2 chopsticks	
			printf("Philosopher %d is preempted by philosopher %d\n", LEFT(i)+1, i+1);
			strcpy(state_label[LEFT(i)],"( P )");
			strcpy(state_label[i],"(|E|)");
			print_philosophers();
		} else {	// left chopstick is available
			printf("Philosopher %d takes chopsticks %d and %d\n",i+1,i+1,RIGHT(i)+1);
			strcpy(state_label[i],"(|E|)");
			print_philosophers();
		}	
    }
}
  
void put_chopsticks(int i){
    down(&mutex);
	if (state[i]==PREEMPTED) 
		// done with the eating but being preempted before putting chopsticks down
		printf("Philosopher %d has enough, back to thinking\n", i+1);
	else
		printf("Philosopher %d puts chopsticks %d and %d down\n",i+1,i+1,RIGHT(i)+1);
	strcpy(state_label[i],"( T )");
	print_philosophers();
	state[i] = THINKING;
    test(RIGHT(i));
	test(LEFT(i));
    up(&mutex);
}

void think(int i) {
	//printf("Philosopher %d is thinking\n", i+1);
	sleep(rand()% R);
}

void eat(int i) {
	int j=0, time=30;			 // 3 seconds to eat
	for (j=0; j<time; j++) {	 // eat in many small steps
		usleep(100000); 		 // 0.1s
		if (state[i]==PREEMPTED) // when being preempted
			down(&preempted);	 // sleep until the chopstick is returned
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