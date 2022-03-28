#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"
#include "dining.h"

#define NPHILOSOPHERS 8

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

        printf("Philosopher %ld is hungry\n", philosopher);
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