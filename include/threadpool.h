#pragma once 

// Inspired and based off of https://programmer.group/c-simple-thread-pool-based-on-pthread-implementation.html
#include <pthread.h>
#include <stdint.h>
#include <tgmath.h>
#include <stdatomic.h>

#include <sys/syscall.h>
#include <sys/types.h>

#define DEFER_UNLOCK __attribute__((cleanup(unlock)))
#define POOL_CLEANUP __attribute__((cleanup(threadpool_cleanup)))
static inline void unlock(void *lock) {
    pthread_mutex_t* mutex = *(pthread_mutex_t**)lock;
    pthread_mutex_unlock(mutex);
}

typedef struct {
    void (*function)(void*);
    void *argument;
} threadpool_task_t;

typedef struct thread_pool {
    pthread_mutex_t lock;
    threadpool_task_t *queue, *hqueue, *tqueue;
    pthread_cond_t notify;
    pthread_t *threads;

    int shutdown;
    atomic_int enqueued;
    size_t nqueue;
    size_t nthreads;
} threadpool_t;

threadpool_t *threadpool_create(size_t nqueue);
void threadpool_submit(threadpool_t *pool, void (*task)(void*), void* args);
void *threadpool_thread(void* vpool);
void threadpool_finalize(threadpool_t *pool);
void threadpool_cleanup(void *vpool);