#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dining.h"

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