#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "threadpool.h"

#define NPHILOSOPHERS 8
#define THINKING 0
#define HUNGRY 1
#define EATING 2


#define BIT(x, i) (((x) & (i)) == (i))
#define LWRAP(i, N) (((i) + (N) - 1) % (N))
#define RWRAP(i, N) (((i) + 1) % (N))

typedef struct philosopher {
    uint32_t state;
    sem_t semaphore;
}philosopher_t;

typedef struct dining_table {
    philosopher_t *philosophers;
    pthread_mutex_t lock;
    size_t nphilosophers;
}dining_table_t;

dining_table_t *dining_create(size_t nphilosophers) {
    dining_table_t *table = calloc(1, sizeof *table);
    pthread_mutex_init(&table->lock, NULL);
    table->nphilosophers = nphilosophers;
    table->philosophers = calloc(nphilosophers, sizeof *table->philosophers);

    for (size_t i = 0 ; i < nphilosophers ; ++i) {
        sem_init(&table->philosophers[i].semaphore, 0, 0);
    }

    return table;
}

void dining_destroy(dining_table_t *table) {
    for (size_t i = 0 ; i < table->nphilosophers ; ++i) {
        sem_destroy(&table->philosophers[i].semaphore);
    }
    free(table->philosophers);
    pthread_mutex_destroy(&table->lock);
    free(table);
}

void dining_inquire(dining_table_t *table, size_t philosopher) {
    if (BIT(table->philosophers[philosopher].state, HUNGRY) &&
       !BIT(table->philosophers[LWRAP(philosopher, table->nphilosophers)].state, EATING) &&  
       !BIT(table->philosophers[RWRAP(philosopher, table->nphilosophers)].state, EATING)) {
           table->philosophers[philosopher].state = EATING;
           sem_post(&table->philosophers[philosopher].semaphore);
    }
}

void dining_acquire_fork(dining_table_t *table, size_t philosopher) {
    pthread_mutex_lock(&table->lock);
    table->philosophers[philosopher].state = HUNGRY;

    // Be careful here. Libc has printf locked, but if not then do use printf_unlocked(3) + mutex
    printf("Philosopher %ld is hungry\n", philosopher);

    dining_inquire(table, philosopher);
    pthread_mutex_unlock(&table->lock);
    sem_wait(&table->philosophers[philosopher].semaphore);
}

void dining_relinquish_fork(dining_table_t *table, size_t philosopher) {
    pthread_mutex_lock(&table->lock);
    table->philosophers[philosopher].state = THINKING;
    dining_inquire(table, LWRAP(philosopher, table->nphilosophers));
    dining_inquire(table, RWRAP(philosopher, table->nphilosophers));
    pthread_mutex_unlock(&table->lock);
}

struct philosopher_args {
    dining_table_t *table;
    size_t philosopher;
};

void philosopher_thread(void *args) {
    struct philosopher_args *pargs = args;
    dining_table_t *table = pargs->table;
    size_t philosopher = pargs->philosopher;

    while (1) {
        // thinking period
        usleep((lrand48() % 400 + 100) * 1000);

        dining_acquire_fork(table, philosopher);

        // eating period
        printf("Philosopher %ld is eating\n", philosopher);
        usleep((lrand48() % 400 + 100) * 1000);
        printf("Philosopher %ld has finished eating\n", philosopher);
    
        dining_relinquish_fork(table, philosopher);
    }
}

int main () {
    // Just fluff, simply using pthread.h directly is as effective.
    threadpool_t *pool POOL_CLEANUP = threadpool_create(NPHILOSOPHERS);
    dining_table_t *table = dining_create(NPHILOSOPHERS);
    struct philosopher_args* args = calloc(NPHILOSOPHERS, sizeof *args);

    for (size_t i = 0 ; i < NPHILOSOPHERS ; ++i) {
        args[i].table = table;
        args[i].philosopher = i;
        threadpool_submit(pool, philosopher_thread, &args[i]);
    }
    
    threadpool_finalize(pool, GRACEFUL_SHUTDOWN);
}