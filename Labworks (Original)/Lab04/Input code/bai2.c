#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 5
#define R 4
#define TRUE 1

int flag[2] = {0, 0};
int turn = 0;

int queue[N];
int inIdx = 0, outIdx = 0, count = 0;

void produce_item(int* pItem);
void enter_item(int item);
int get_item();
void consume_item(int item);

int main() {
	pthread_t pid, cid;

	pthread_create(&pid, NULL, producer, NULL);
	pthread_create(&cid, NULL, consumer, NULL);
	pthread_join(pid, NULL);
	pthread_join(cid, NULL);

	return 0;
}

void lock(int i) {
    flag[i] = 1;
    turn = 1 - i;
    while (turn == 1 - i && flag[1 - i] == 1);
}

void unlock(int i) {
    flag[i] = 0;
}

void* producer(void* arg) {
    int item;

    printf("P: Hello\n");
    while (1) {
        produce_item(&item);

        while (count == N); 

        lock(0);
        enter_item(item);
        unlock(0);

        sleep(rand() % R);
    }
    return NULL;
}

void* consumer(void* arg) {
    int item;
    printf("C: Hello\n");

    while (1) {
        while (count == 0); 

        lock(1);
        item = get_item();
        unlock(1);

        consume_item(item);
        sleep(rand() % R);
    }
    return NULL;
}

void produce_item(int* pItem) {
	*pItem = rand() % 100;
	printf("P: Produce item %d\n", *pItem);
}

void enter_item(int item) {
	queue[inIdx] = item;
	inIdx = (inIdx+1) % N;
	count++;
	printf("P: Enter item %d\n", item);
	if (count == N ) printf("P: The queue is full\n");
}

int get_item() {
	int item = queue[outIdx];
	outIdx = (outIdx+1) % N;
	count--;
	printf("C: Get item %d\n", item);
	if (count == 0) printf("C: The queue is empty\n");

	return item;
}

void consume_item(int item) {
	printf("C: Item %d is yum yum!\n", item);
}

