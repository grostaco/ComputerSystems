#include "threadpool.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

threadpool_t *threadpool_create(size_t nqueue) {
    threadpool_t* pool = calloc(sizeof *pool, 1);
    
    
    pool->nthreads = sysconf(_SC_NPROCESSORS_ONLN);
    pool->nqueue = nqueue;

    pool->threads = calloc(pool->nthreads, sizeof *pool->threads);
    pool->queue = calloc(nqueue, sizeof *pool->queue);
    pool->hqueue = pool->tqueue = pool->queue;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notify, NULL);

    for (size_t i = 0 ; i < pool->nthreads ; ++i) {
        pthread_create(&pool->threads[i], NULL, threadpool_thread, pool);
    }

    return pool;
}

void threadpool_submit(threadpool_t *pool, void (*task)(void*), void* args) {
    pthread_mutex_t *lock DEFER_UNLOCK = &pool->lock;
    pthread_mutex_lock(lock);

    threadpool_task_t *next = pool->tqueue + 1;
    if (next == pool->queue + pool->nqueue) {
        next = pool->queue;
    }

    if (pool->occupied == pool->nqueue) {
        return;
    }

    if (pool->shutdown) {
        return;
    }

    (*pool->tqueue).function = task;
    (*pool->tqueue).argument = args;

    pool->tqueue = next;
    ++pool->enqueued;
    pthread_cond_signal(&pool->notify);
}

void *threadpool_thread(void* vpool) {
    threadpool_t *pool = vpool;
    threadpool_task_t task;
    pthread_mutex_t *lock DEFER_UNLOCK = &pool->lock;

    while (1) {
        pthread_mutex_lock(lock);

        while ((pool->enqueued == 0) && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, lock);
        }

        if (pool->shutdown) {
            break;
        }
           
        task.function = (*pool->hqueue).function;
        task.argument = (*pool->hqueue).argument;

        ++pool->hqueue;
        --pool->enqueued;

        if (pool->hqueue == pool->queue + pool->nqueue) {
            pool->hqueue = pool->queue;
        }
        
        // printf("Unlocked\n");
        pthread_mutex_unlock(lock);
        task.function(task.argument);
    }   

    pthread_mutex_unlock(&pool->lock);
    pthread_exit(NULL);
}

void threadpool_finalize(threadpool_t *pool) {
    while (pool->enqueued != 0);
    pthread_mutex_lock(&pool->lock);

    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

    for (size_t i = 0 ; i < pool->nthreads ; ++i) {
        pthread_join(pool->threads[i], NULL);
    }
}

void threadpool_cleanup(void *vpool) {
    threadpool_t *pool = *(void**)vpool;
    free(pool->threads);
    free(pool->queue);

    pthread_mutex_lock(&pool->lock);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);

    free(pool);
}