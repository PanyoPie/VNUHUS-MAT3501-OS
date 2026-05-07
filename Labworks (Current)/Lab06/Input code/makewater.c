// Make H2O
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
  
#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)
#define DELAY(R) sleep(rand() % R)
  
sem_t h_sem, o_sem, mutex;
int	w_molecules;
  
void* make_hydrogen(void *num);
void* make_oxygen(void *num);
void* make_water(void *num);
  
int main()
{
    int i;
    pthread_t h_thread_id, o_thread_id, w_thread_id;
	
    sem_init(&h_sem,0,0);
	sem_init(&o_sem,0,0);
	sem_init(&mutex,0,1);
	
    pthread_create(&w_thread_id,NULL,make_water,NULL);
	pthread_create(&h_thread_id,NULL,make_hydrogen,NULL);
	pthread_create(&o_thread_id,NULL,make_oxygen,NULL);
	
    pthread_join(o_thread_id,NULL);
	pthread_join(h_thread_id,NULL);
	pthread_join(w_thread_id,NULL);
}
  
void* make_hydrogen(void *num)
{
    while(1) {
        DELAY(3);
		...		// acquire mutex
		...		// create a hydrogen atom
		...		// printf " H "
		...		// realease mutex
    }
}

void* make_oxygen(void *num)
{
    while(1) {
        DELAY(4);
		...		// acquire mutex
		...		// create an oxygen atom
		...		// printf " O "
		...		// realease mutex
    }
}

void* make_water(void *num)
{
    while(1) {
		...		// get 2 hydrogen atoms
		...		// get 1 oxygen atom

		...		// acquire mutex
		...		// increase number of water molecules produced
		...		// printf " H2O#%d\n", where %d is the number of water molecules produced
		...		// release mutex
        sleep(1);
    }
}
