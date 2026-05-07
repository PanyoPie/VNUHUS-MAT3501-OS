/* Reader-Writer problem using monitor, with writer starvation 
 * This version uses a user-defined "monitor" library mesa_sync.c
 * ========================================================================================
 * Compiler: gcc -o -pthread reader_writer_mymonitor reader_writer_mymonitor.c mesa_sync.c
 * ========================================================================================
 */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "mesa_sync.h"
/* uncomment this to use the system library mutex & cond var
#define my_cond_t	pthread_cond_t
#define my_mutex_t	pthread_mutex_t

#define my_mutex_init		pthread_mutex_init
#define my_mutex_lock		pthread_mutex_lock
#define my_mutex_unlock		pthread_mutex_unlock

#define my_cond_init		pthread_cond_init
#define my_cond_wait		pthread_cond_wait
#define my_cond_signal		pthread_cond_signal
#define my_cond_broadcast	pthread_cond_broadcast
*/
#define R 2
#define DELAY	sleep(rand() % R);

int readerCount;		// no. of readers
int writerCount;		// no. of writers
int waitingReaders;		// no. of readers waiting

my_cond_t canRead;		// whether reader can read
my_cond_t canWrite;		// whether writer can write
my_mutex_t mutex;		// mutex for synchronization

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
	
	// 
	readerCount = 0;
	writerCount = 0;
	waitingReaders = 0;
	my_cond_init(&canRead, NULL);
	my_cond_init(&canWrite, NULL);
	my_mutex_init(&mutex, NULL);
		
	noReaderThreads = 10; assert(noReaderThreads<100);
	noWriterThreads = 3;  assert(noWriterThreads<100);
 
	for(i=0;i<noWriterThreads;i++) 
		pthread_create(&writerThread[i],NULL,writer,(void *)(intptr_t)i);
	
	for(i=0;i<noReaderThreads;i++)
		pthread_create(&readerThread[i],NULL,reader,(void *)(intptr_t)i);
	
	for(i=0;i<noWriterThreads;i++)
		pthread_join(writerThread[i],NULL);

	for(i=0;i<noReaderThreads;i++)
		pthread_join(readerThread[i],NULL); 

	return 0;
}

void * writer(void *arg) {
	int id=(int)(intptr_t)arg;
	
	DELAY;	
	
	beginWrite(id);
	writeData(id);	// write to the database
	endWrite(id);
}

void *reader(void *arg) { 
	int id=(int)(intptr_t)arg;
	
	DELAY;
	
	beginRead(id);
	readData(id);	// read the database
	endRead(id);
}

void beginRead(int i) {
	my_mutex_lock(&mutex);

	// if there are active or waiting writers
	if (writerCount == 1) {
		// incrementing waiting readers
		waitingReaders++;

		// reader suspended
		my_cond_wait(&canRead, &mutex);
		waitingReaders--;
	}

	// else reader reads the resource
	readerCount++;
	my_mutex_unlock(&mutex);
	my_cond_broadcast(&canRead);
}

void endRead(int i) {
	// if there are no readers left then writer enters monitor
	my_mutex_lock(&mutex);

	if (--readerCount == 0)
		my_cond_signal(&canWrite);

	my_mutex_unlock(&mutex);
}

void beginWrite(int i) {
	my_mutex_lock(&mutex);

	// a writer can enter when there are no active
	// or waiting readers or other writer
	if (writerCount == 1 || readerCount > 0) 
		my_cond_wait(&canWrite, &mutex);
	writerCount = 1;
	my_mutex_unlock(&mutex);
}

void endWrite(int i) {
	my_mutex_lock(&mutex);
	writerCount = 0;

	// if any readers are waiting, threads are unblocked
	if (waitingReaders > 0)
		my_cond_signal(&canRead);
	else
		my_cond_signal(&canWrite);
	my_mutex_unlock(&mutex);
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