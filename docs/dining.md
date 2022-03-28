# Dining philosopher 

A dining philosopher problem solved using Dijkstra's solution with semaphores.

# ToC
- [API](#api)
    - [Structs](#structs)
    - [Functions](#functions)
- [Implementation](#implementation)
    - [Core problem](#core-problem)
    - [Helper functions](#helper-functions)
- [Examples](#examples)
    - [Dining philosophers problem](#dining-philosophers-problem)
# API
## Structs
```c
typedef struct philosopher {
    uint32_t state;
    sem_t semaphore;
} philosopher_t;
```
| field | description |
|-------|------------|
| state | The current state of the philosopher (thinking, hungry, eating) |
| semaphore     | The philosopher's semaphore |

```c
typedef struct dining_table {
    philosopher_t *philosophers;
    pthread_mutex_t lock;
    size_t nphilosophers;
} dining_table_t;
```
| field | description |
|-------|------------|
| philosophers | Philosophers in the dining table |
| lock     | Lock for critical regions |
| nphilosophers | How many philosophers in the table |

## Functions

```c
dining_table_t *dining_create(size_t nphilosophers);
```
Create a table with `nphilosophers` philosophers.

```c
void dining_destroy(dining_table_t *table);
```
Free all resources allocated by `table`. 

```c
void dining_inquire(dining_table_t *table, size_t philosopher);
```
Internal function. Used to check whether two forks can be acquired by inquiring neighboring philosophers on their states.

```c
void dining_acquire_fork(dining_table_t *table, size_t philosopher);
```
Internal function. Attempt to acquire a pair of fork from neighboring philosophers, block until a fork is available.

```c
void dining_relinquish_fork(dining_table_t *table, size_t philosopher);
```
Internal function. Release the control of the fork pair and tell neighboring philosophers to inquire their neighboring philosopher states to acquire forks.

# Implementation
## Core problem
The difficulty in implementing a philosopher's dining table is in causing a deadlock. To circumvent this, we adopt Dijkstra's solution to this problem.
The diner will behave as follows

- The diner will spend an arbitrary amount of time thinking
- The diner will __acquire__ a fork
    - To acquire a fork, the philosopher will set its own state to `HUNGRY`
        - This state allows other diners to assist in helping the diner out of this state
    - Try to __inquire__ neighboring diners for their forks
        - If at least one is eating, return 
        - Otherwise set the diner's state to `EATING` and increment their semaphore counter
    - Wait on the semaphore
        - If the __inquire__ process was successful, this would return immediately
        - Otherwise wait until a neighboring diner assists them
- The diner will spend an arbitrary amount of time eating
- The diner will __relinquish__ their control of the fork
    - The diner will set their own state to `THINKING`
        - This state helps neighboring diners to not assist this diner in finding forks
    - Assist neighboring diners that are `HUNGRY` by __inquiring__ their neighbors for forks in their stead

A semaphore allows other diners to help in finding forks for the diner owning the semaphore, as well as waiting on the semaphore until assistance is given.

## Helper functions
To implement above, we shall turn the process into a function called `philosopher_thread`.

```c
struct philosopher_args {
    dining_table_t *table;
    size_t philosopher;
};

void philosopher_thread(void *args) {
    // Due to lack of generics
    struct philosopher_args *pargs = args;
    dining_table_t *table = pargs->table;
    size_t philosopher = pargs->philosopher;

    // Cycle of thinking, being hungry, and eating.
    while (1) {
        // thinking period
        usleep((lrand48() % 400 + 100) * 1000);

        printf("Philosopher %ld is hungry\n", philosopher);
        // Try to acquire forks
        dining_acquire_fork(table, philosopher);

        // eating period, forks acquired
        printf("Philosopher %ld is eating\n", philosopher);
        usleep((lrand48() % 400 + 100) * 1000);
        printf("Philosopher %ld has finished eating\n", philosopher);

        // Relinquish the forks and assist neighboring diners
        // in inquiring their neighboring diners for forks
        dining_relinquish_fork(table, philosopher);
    }
}
```

# Examples
## Dining philosophers problem
```c
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
```
A dining philosophers problem with 8 diners. 
Possible outputs:
```
$ make clean run
...
./bin/out 
Philosopher 0 is hungry
Philosopher 0 is eating
Philosopher 6 is hungry
Philosopher 6 is eating
Philosopher 0 has finished eating
Philosopher 1 is hungry
Philosopher 1 is eating
Philosopher 4 is hungry
Philosopher 4 is eating
Philosopher 5 is hungry
Philosopher 7 is hungry
Philosopher 0 is hungry
Philosopher 2 is hungry
Philosopher 3 is hungry
Philosopher 4 has finished eating
Philosopher 3 is eating
Philosopher 6 has finished eating
Philosopher 5 is eating
Philosopher 7 is eating
Philosopher 1 has finished eating
```

## Discussion
We will outline the process justifying the output above.
- Philosopher 0 is hungry & eating
    - As no neighboring philosophers are eating, philosopher 0 starts eating
- Philosopher 6 is hungry & eating
    - As no neighboring philosophers are eating, philosopher 0 starts eating
- Philosopher 0 finished eating
- Philosopher 1 is hungry & eating
    - As philosopher 0 just finished eating, no neighboring philosophers are eating. Philosopher 1 can start eating
- Philosopher 4 is hungry & eating
    - As no neighboring philosophers are eating, philosopher 0 starts eating
- Philosopher 5 is hungry
    - As philosopher 4 is eating, philosopher 5 cannot eat
- Philosopher 7 is hungry
    - As philosopher 6 is eating, philosopher 7 cannot eat
- Philosopher 0 & 2 are hungry
    - As philosopher 1 is eating, philosopher 0 & 2 cannot eat
- Philosopher 3 is hungry
    - As philosopher 4 is eating, philosopher 3 cannot eat
- Philosopher 4 has finished eating
- Philosopher 3 is eating
    - As philosopher 4 has finished eating, philosopher 3 can eat
- Philosopher 6 has finished eating
- Philosopher 5 & 7 are eating
    - As philosopher 4 & 6 has finished eating, philosopher 5 and 7 can eat.
- Philosopher 1 has finished eating