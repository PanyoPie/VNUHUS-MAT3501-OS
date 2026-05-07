/* Dining philosophers using semaphore, breaking Mutual Exclusion condition:  
   Create an arbiter thread to maintain a queue of chopstick requests and release messages
   Philosophers have to send a message to the arbiter to request/release chopsticks
		A message contains the philosopher ID and a message type of REQUEST/RELEASE chopsticks
		REQUEST message is added to the end of the queue
		RELEASE message is urgent and inserted to the head of the queue
   Semaphore arbiter_notifications is used to synchorize between philosophers and the arbiter
   Upon an arrived message, the arbiter get it from the queue
		if it's a chopstick request, test for the philosopher
		if it's a chopstick release, test for the left and right philosophers
   test() function checks if 2 chopsticks are available then raises up semaphore S[i]
   The philosopher calls down(&S[i]) to take both chopsticks or otherwise sleep waiting
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
  
 
sem_t arbiter_notifications; // request/release messages to the arbiter
sem_t mutex;			// mutex is used for queue access and printing purpose
sem_t S[N];				// S[i] represents whether philosopher i can eat

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
	sem_init(&arbiter_notifications,0,0);
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
The arbiter get a message from the queue
	if it's a chopstick request, test for the philosopher
	if it's a chopstick release, test for the left & right philosophers
*/
void *arbiter(void *pArg) {
	int id;
	char ch;
	MessageType_t type;
	while (1) {
		down(&arbiter_notifications);	// get a request or sleep
		down(&mutex);	// ----------------------------------
		get_queue_message(&id, &type);
		up(&mutex);		// ----------------------------------
		
		if(type==REQUEST) // request message
			test(id);
		else {	// release message
			test(LEFT(id));
			test(RIGHT(id));
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
	down(&mutex);		// ----------------------------------
	state[i] = HUNGRY;
	strcpy(state_label[i],"( H )");
	printf("Philosopher %d is asking for chopsticks\n",i+1);
	//print_philosophers();
	add_queue_request(i, REQUEST);
	print_philosophers();
	up(&mutex);			// ----------------------------------
	up(&arbiter_notifications);
	
    down(&S[i]);
    sleep(1);
}

/*
Inform the arbiter when chopsticks are released.
*/
void arbiter_release(int i) {
	down(&mutex);		// ----------------------------------
	state[i] = THINKING;
	strcpy(state_label[i],"( T )");
	printf("Philosopher %d puts chopsticks down\n",i+1);
	//print_philosophers();
	insert_queue_release(i, RELEASE);	
	print_philosophers();
	up(&mutex);			// ----------------------------------
	up(&arbiter_notifications);
	
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
		strcpy(state_label[i],"(|E|)");
        up(&S[i]);
		
		printf("Arbiter let philosopher %d eat\n", i+1);
		print_philosophers();
    }
	else if (state[i]==HUNGRY) 
		printf("Arbiter cannot fulfill philosopher %d\n", i+1);
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
	print_queue();
	printf("\n");
}