/* Reader-Writer using semaphore, no writer starvation */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define up(sem)	sem_post(sem)
#define down(sem)	sem_wait(sem)
#define R 2
#define DELAY	sleep(rand() % R);

sem_t readCountAccess;
sem_t databaseAccess;
sem_t fifo;
int readCount=0;

void *reader(void *arg);
void *writer(void *arg);
void readData(int i);
void writeData(int i);

int main() {
	unsigned int i,noReaderThreads,noWriterThreads;
	pthread_t readerThread[100],writerThread[100];
	
	sem_init(&readCountAccess,0,1);
	sem_init(&databaseAccess,0,1);
	sem_init(&fifo,0,1);
		
	noReaderThreads = 10; assert(noReaderThreads<100);
	noWriterThreads = 5;  assert(noWriterThreads<100);
 
	for(i=0;i<noWriterThreads;i++)
		pthread_create(&writerThread[i],NULL,writer,(void *)i);
	
	for(i=0;i<noReaderThreads;i++) 
		pthread_create(&readerThread[i],NULL,reader,(void *)i);
	
	for(i=0;i<noWriterThreads;i++) 
		pthread_join(writerThread[i],NULL);

	for(i=0;i<noReaderThreads;i++)
		pthread_join(readerThread[i],NULL); 
	
	sem_destroy(&databaseAccess);
	sem_destroy(&readCountAccess); 
	sem_destroy(&fifo); 
	return 0;
}

void * writer(void *arg){
	int id=(int)arg;
	
	DELAY;
	
	// begin write
	...
	down(&databaseAccess);
	...

	writeData(id);	// write to the database
	
	// end write
	up(&databaseAccess);
}

void *reader(void *arg) { 
	int id=(int)arg;

	DELAY;

	// begin read	
	...
	down(&readCountAccess);
	readCount++;
	if(readCount==1) down(&databaseAccess);
	up(&readCountAccess);
	...
	
	readData(id);	// read the database
	
	// end read
	down(&readCountAccess);
	readCount--;
	if(readCount==0) up(&databaseAccess);
	up(&readCountAccess);
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