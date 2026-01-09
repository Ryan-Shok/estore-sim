#ifndef _STHREAD_H_
#define _STHREAD_H_
/* 
Note: this library requires you to link with the posix threads
library (-lpthread) and the real time library (-lrt) {for
nanosleep}.

   gcc -D_POSIX_PTHREAD_SEMANTICS main.c sthread.c -lpthread -lrt
*/

#include <pthread.h>
#include <unistd.h>

typedef pthread_mutex_t smutex_t;
typedef pthread_cond_t scond_t;
typedef pthread_t sthread_t;

void smutex_init(smutex_t *mutex);
void smutex_destroy(smutex_t *mutex);
void smutex_lock(smutex_t *mutex);
void smutex_unlock(smutex_t *mutex);

void scond_init(scond_t *cond);
void scond_destroy(scond_t *cond);


void scond_signal(scond_t *cond, smutex_t *mutex);
void scond_broadcast(scond_t *cond, smutex_t *mutex);
void scond_wait(scond_t *cond, smutex_t *mutex);



void sthread_create(sthread_t *thrd,
                    void *(start_routine(void*)), 
                    void *argToStartRoutine);
void sthread_exit(void);

/*
 * Block until the specified thread exits. If the thread has
 * already exited, this function returns immediately.
 */
void sthread_join(sthread_t thrd);


void sthread_sleep(unsigned int seconds, unsigned int nanoseconds);


/*
 * The normal random() library is not thread safe,
 * so we add a wrapper with locks.
 */
long sutil_random(void);

#endif

