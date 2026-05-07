#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define N 5
#define TRUE 1
#define DELAY(R)	sleep(rand() % R);

pthread_cond_t empty, full; 
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER; 
	// uses this macro to initialize default value to static mutex

int queue[N];
int inIdx, outIdx, count;

void produce_item(int* pItem) {
	(*pItem)=rand() % 100; // generate a random integer upto 99
	printf("P: Produce item %d\n", *pItem);
}

void enter_item(int item) {
	queue[inIdx]=item;
	inIdx = (inIdx+1) % N;
	count++;
	printf("P: Enter item %d\n", item);
	if (count==N) printf("P: The queue is full\n");
}

int get_item() {
	int item;
	item=queue[outIdx];
	outIdx = (outIdx+1) % N;
	count--;
	printf("C: Get item %d\n", item);
	if (count==0) printf("C: The queue is empty\n");

	return item;
}

void consume_item(int item) {
	printf("C: Item %d is yum yum!\n", item);
}

void * producer (void * arg) {
	int item;
	printf("P: Hello\n");
	while (TRUE) {
		produce_item(&item);					// produce an item

		pthread_mutex_lock(&mutex);		// acquire the lock
		while (count == N)	// check if the buffer is full
			pthread_cond_wait(&empty, &mutex);	// wait for buffer to be emptied
		enter_item(item);					// enter item to buffer
		pthread_cond_signal(&full);		// signal that the buffer is full
		pthread_mutex_unlock(&mutex);		// release the lock

		DELAY(3);
	}
}

void * consumer (void * arg) {
	int item;
	printf("C: Hello\n");
	while (TRUE) {
		pthread_mutex_lock(&mutex);		// acquire mutex lock
		while (count == 0)	// check if the buffer is empty
			pthread_cond_wait(&full, &mutex);	// wait for the buffer to be filled
		item = get_item();					// get item from buffer
		pthread_cond_signal(&empty);		// signal that the buffer is empty
		pthread_mutex_unlock(&mutex);		// release mutex lock
		
		consume_item(item);					// consume the item
		DELAY(3);
	}
}

int main(int argc, char * argv[]) {
	pthread_t pid,cid;

	pthread_create(&pid,NULL,producer,NULL);
	pthread_create(&cid,NULL,consumer,NULL);
	pthread_join(pid,NULL);
	pthread_join(cid,NULL);
	return 0;
}

