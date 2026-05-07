/* mesa_sync.c
 * Mesa "monitor" implementation using semaphores.
 * As C is not OOP, so "monitors" actually do not exist. Instead
 * this module implements mutex locks and condition variables using 
 * semaphores following Mesa (signal-and-continue) semantics.
 * This module can be used as a replacement for the condition variable /
 * mutex functions of the POSIX Threads (pthreads) library
 *
 * Mesa semantics means:
 *   - A signaling thread CONTINUES running after calling my_cond_signal().
 *   - The woken thread is placed in the ready queue, but must re-check
 *     the condition predicate (hence callers always use a while-loop).
 *   - This is the semantics used by POSIX pthreads (pthread_my_cond_signal).
 *
 * Compile: gcc -o <yourprog> <yourprog.c> mesa_sync.c -lpthread
 */

#include <semaphore.h>
#include "mesa_sync.h"

#define up(sem) sem_post(sem)
#define down(sem) sem_wait(sem)

/* =========================================================================
 * Mutex lock functions
 * ========================================================================= */

/* Initialize the mutex to the "unlocked" state (semaphore value = 1). 
 * 2nd argument attr is not used, declared for compatibility with the original pthread function
 */
void my_mutex_init(my_mutex_t *m, void *attr) {
    sem_init(&m->sem, 0 /* shared among threads */, 1 /* initial value */);
}

/* Acquire the mutex.
 * down() atomically decrements the semaphore.
 * If the value is already 0 (locked), the caller blocks until it becomes 1. */
void my_mutex_lock(my_mutex_t *m) {
    down(&m->sem);
}

/* Release the mutex.
 * up() atomically increments the semaphore, unblocking one waiter. */
void my_mutex_unlock(my_mutex_t *m) {
    up(&m->sem);
}

/* Destroy the mutex, releasing kernel resources. */
void my_mutex_destroy(my_mutex_t *m) {
    sem_destroy(&m->sem);
}


/* =========================================================================
 * Condition variable functions
 * ========================================================================= */

/* Initialize the condition variable.
* 2nd argument attr is not used, declared for compatibility with the original pthread function
 */
void my_cond_init(my_cond_t *cv, void *attr) {
    cv->waiter_count = 0;			// number of waiting threads is 0
    sem_init(&cv->wait_sem, 0, 0);	// threads start blocked on this
    sem_init(&cv->guard,    0, 1);	// internal guard starts unlocked
}

/*
 * my_cond_wait -- atomically release the mutex and sleep until signaled.
 *
 * Steps (Mesa semantics):
 *  1. Lock the internal guard to safely increment 'waiter_count'.
 *  2. Increment the waiter count (we are about to sleep).
 *  3. Release the caller's mutex so other threads can make progress.
 *  4. Unlock the guard.
 *  5. Block on wait_sem (the actual sleep).
 *  6. On wake-up, re-acquire the caller's mutex before returning.
 *     (The caller must re-check the predicate in a while-loop because
 *      another thread may have changed the state between the signal and
 *      this thread's re-acquisition of the mutex — the essence of Mesa.)
 */
void my_cond_wait(my_cond_t *cv, my_mutex_t *m) {
    /* --- Step 1 & 2: register as a waiter under the guard --- */
    down(&cv->guard);
    cv->waiter_count++;
    up(&cv->guard);

    /* --- Step 3: release the mutex so others can run --- */
    my_mutex_unlock(m);

    /* --- Step 4: guard already posted above, sleep now --- */
    down(&cv->wait_sem);   // <-- thread sleeps here

    /* --- Step 5 (after wake): re-acquire the caller's mutex ---
     * Mesa: we compete with all other threads for the mutex.
     * The condition predicate must be rechecked by the caller. */
    my_mutex_lock(m);
}

/*
 * my_cond_signal -- wake ONE waiting thread (if any).
 *
 * The signaler continues to run (Mesa / signal-and-continue).
 * The woken thread will compete for the mutex when it unblocks.
 */
void my_cond_signal(my_cond_t *cv) {
    down(&cv->guard);          // protect the waiter count
    if (cv->waiter_count > 0) {
        cv->waiter_count--;    // one waiter will be unblocked
        up(&cv->wait_sem);     // wake exactly one sleeper
    }
    up(&cv->guard);            // release guard

    /*
     * Mesa note: we do NOT release the caller's mutex here.
     * The signaling thread keeps the mutex and keeps running.
     * The woken thread blocks in my_mutex_lock() until the signaler
     * (or someone else) eventually calls my_mutex_unlock().
     */
}

/*
 * my_cond_broadcast -- wake ALL waiting threads.
 *
 * Each woken thread still has to re-acquire the mutex and re-check
 * the predicate (Mesa semantics applies equally to broadcast).
 */
void my_cond_broadcast(my_cond_t *cv) {
    down(&cv->guard);
    while (cv->waiter_count > 0) {
        cv->waiter_count--;
        up(&cv->wait_sem);     // wake one per iteration
    }
    up(&cv->guard);
}

/* Destroy condition variable resources. */
void my_cond_destroy(my_cond_t *cv) {
    sem_destroy(&cv->wait_sem);
    sem_destroy(&cv->guard);
}
