/* Producers - Consumers using monitor, no writer starvation */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define R 2
#define DELAY	sleep(rand() % R);

int readerCount;		// no. of readers
int writerCount;		// no. of writers
int waitingReaders;		// no. of readers waiting
int waitingWriters;		// no. of writers waiting


pthread_cond_t canRead;		// whether reader can read
pthread_cond_t canWrite;	// whether writer can write
pthread_mutex_t mutex;		// mutex for synchronization

void *reader(void *arg);
void *writer(void *arg);
void beginRead(int i);
void beginWrite(int i);
void endRead(int i);
void endWrite(int i);
void readData(int i);
void writeData(int i);

int main() {
	unsigned int i,noReaderThreads,noWriterThreads;
	pthread_t readerThread[100],writerThread[100];
	
	// Initialization
	readerCount = 0;
	writerCount = 0;
	waitingReaders = 0;
	waitingWriters = 0;
	pthread_cond_init(&canRead, NULL);
	pthread_cond_init(&canWrite, NULL);
	pthread_mutex_init(&mutex, NULL);
		
	noReaderThreads = 10; assert(noReaderThreads<100);
	noWriterThreads = 3;  assert(noWriterThreads<100);
 
	for(i=0;i<noWriterThreads;i++) 
		pthread_create(&writerThread[i],NULL,writer,(void *)i);
	
	for(i=0;i<noReaderThreads;i++)
		pthread_create(&readerThread[i],NULL,reader,(void *)i);
	
	for(i=0;i<noWriterThreads;i++)
		pthread_join(writerThread[i],NULL);

	for(i=0;i<noReaderThreads;i++)
		pthread_join(readerThread[i],NULL); 

	return 0;
}

void * writer(void *arg) {
	int id=(int)arg;
	
	DELAY;
	
	beginWrite(id);
	writeData(id);	// write to the database
	endWrite(id);
}

void *reader(void *arg) { 
	int id=(int)arg;

	DELAY;
	
	beginRead(id);
	readData(id);	// read the database
	endRead(id);
}

void beginRead(int i) {
	pthread_mutex_lock(&mutex);

	// if there are active or waiting writers
	if (writerCount == 1 || waitingWriters > 0) {
		// incrementing waiting readers
		waitingReaders++;

		// reader suspended
		pthread_cond_wait(&canRead, &mutex);
		waitingReaders--;
	}

	// else reader reads the resource
	readerCount++;
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&canRead);
}

void endRead(int i) {
	// if there are no readers left then writer enters monitor
	pthread_mutex_lock(&mutex);

	if (--readerCount == 0)
		pthread_cond_signal(&canWrite);

	pthread_mutex_unlock(&mutex);
}

void beginWrite(int i) {
	pthread_mutex_lock(&mutex);

	// a writer can enter when there are no active
	// or waiting readers or other writer
	if (writerCount == 1 || readerCount > 0) {
		waitingWriters++;
		pthread_cond_wait(&canWrite, &mutex);
		waitingWriters--;
	}
	writerCount = 1;
	pthread_mutex_unlock(&mutex);
}

void endWrite(int i) {
	pthread_mutex_lock(&mutex);
	writerCount = 0;

	// if any readers are waiting, threads are unblocked
	if (waitingReaders > 0)
		pthread_cond_signal(&canRead);
	else
		pthread_cond_signal(&canWrite);
	pthread_mutex_unlock(&mutex);
}

void readData(int i) {
	printf("Reader %d is reading\n", i);
	DELAY;
	printf("Reader %d is done with the reading\n", i);
}

void writeData(int i) {
	printf("Writer %d is writing\n", i);
	DELAY;
	printf("Writer %d is done with the writing\n", i);
}