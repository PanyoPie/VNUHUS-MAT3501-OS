/* mesa.h
 * Mesa "monitor" implementation using semaphores. 
 */
#ifndef MESA_SYNC_H
#define MESA_SYNC_H

#include <semaphore.h>

#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)

/* =========================================================================
 * my_mutex_t a binary semaphore type for "monitor" mutex lock
 * ========================================================================= */
typedef struct {
    sem_t sem;   // binary semaphore: 1 = unlocked, 0 = locked
} my_mutex_t;

/* mutex lock functions */
void my_mutex_init(my_mutex_t *m, void *attr);
void my_mutex_lock(my_mutex_t *m);
void my_mutex_unlock(my_mutex_t *m);
void my_mutex_destroy(my_mutex_t *m);

/* =========================================================================
 * my_cond_t condition variable type
 * ========================================================================= */
typedef struct {
    int       waiter_count; // count of threads currently sleeping on this condition var
    sem_t     wait_sem;		// semaphore on which waiting threads block
    sem_t     guard;		// binary semaphore protecting the waiter counter
} my_cond_t;

/* condition variable functions */
void my_cond_init(my_cond_t *cv, void *attr);
void my_cond_wait(my_cond_t *cv, my_mutex_t *m);
void my_cond_signal(my_cond_t *cv);
void my_cond_broadcast(my_cond_t *cv);
void my_cond_destroy(my_cond_t *cv);
    
#endif /* MESA_SYNC_H */