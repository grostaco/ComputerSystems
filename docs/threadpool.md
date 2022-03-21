# Thread Pool
A simple thread pool implementation inspired by [om.bitsian's implementation](https://programmer.group/c-simple-thread-pool-based-on-pthread-implementation.html).

# ToC
- [API](#api)
    - [Thread pool structs](#thread-pool-structs)
    - [Thread pool functions](#thread-pool-functions)
- [Implementation](#implementation)
    - [Thread pools](#thread-pools)
        - [Creation](#creation)
        - [Submission](#submission)
        - [Finalization](#finalization)
- [Examples](#examples)
    - [Simple sqrt program](#simple-sqrt-program)
    - [Example program](#example-program)
- [Disclaimer](#disclaimer)

# API 
## Thread pool Structs
```c
typedef struct {
    void (*function)(void*);
    void *argument;
} threadpool_task_t;
```
| field | description |
|-------|-------------|
| function | function to be called |
| argument | argument to be passed to this function |

```c
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
```
| field | description |
|-------|------------|
| lock | Lock for pushing new tasks and critical sections |
| queue | Queue of all pushed tasks |
| hqueue | Current head of the queue |
| tqueue | Current tail of the queue |
| notify | Conditional block for worker threads |
| threads | Array to maintain thread list |
| shutdown | Flag to notify the worker threads to exit |
| enqueued | How many tasks are currently on queue |
| nqueue | The maximum capacity of `queue` |
| nthreads | How many threads were spawned |

## Thread pool functions
```c
threadpool_t *threadpool_create(size_t nqueue);
```
Create a thread pool which can take up to `nqueue` tasks.

```c
void threadpool_submit(threadpool_t *pool, void (*task)(void*), void* args);
```
Submit a task to the thread pool.

```c
void threadpool_finalize(threadpool_t *pool, int flags);
```
If `flags` is `IMMEDIATE_SHUTDOWN`, request every threads in the pool to exit immediately; otherwise if `flags` is `GRACEFUL_SHUTDOWN`, wait until the task queue is depleted.

```c
void threadpool_cleanup(void *vpool);
```
Helper function to free pool resources. Should never be called directly; instead use `POOL_CLEANUP` macro.

```c
void *threadpool_thread(void* vpool);
```
Helper function to take new tasks. Should never be called directly.

# Implementation 
## Thread pools
### Creation 
`threadpool_create` uses `sysconf(_SC_NPROCESSORS_ONLN)` to choose the most optimal number of threads (one thread per core). The function is a simple allocation function to initialize mutexes, conditional variables, and create threads.

### Submission
`threadpool_submit` will acquire a lock from `threadpool` and once acquired, will conditionally wait until threadpool's `notify` has been notified or broadcasted if there is nothing in the task queue, else it will take a task from the task queue and execute it.  

### Finalization
`threadpool_finalize` simply waits until threadpool's `enqueued` is 0 then request a shutdown if `GRACEFUL_SHUTDOWN` was passed; otherwise immediately make all threads in the pool exit.

# Examples

## Simple sqrt program
```c
#include <stdio.h>

#include "threadpool.h"

void print_sqrt(void* args) {
    printf("%f\n", sqrt(*(double*)args));
}

int main (void) {
    double d = 4.;
    // Create a thread pool with maximum queue size of 4
    threadpool_t *pool POOL_CLEANUP = threadpool_create(4);

    // push work onto the thread pool's queue
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);

    // wait until the queue is depleted
    threadpool_finalize(pool, GRACEFUL_SHUTDOWN);
    return 0;
}
```
Output:
```sh
make clean run
...
./bin/out 
2.000000
2.000000
2.000000
2.000000
```

## Example program
```c
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>

#include "threadpool.h"

#define min(a, b) ((a) < (b)?(a):(b))

struct accumulate_args {
    uint32_t *vals;
    size_t nvals;
    uint64_t acc;
};

void accumulate(void* vargs) {
    struct accumulate_args *args = vargs;
    for (size_t i = 0 ; i < args->nvals ; ++i) {
        args->acc += args->vals[i];
    }
}

int main (void) {
    size_t n;
    size_t splits;
    uint32_t *vals;

    do {
        __fpurge(stdin);
        printf("How many numbers do you want to add? ");
    }
    while (scanf(" %ld", &n) == 0);

    do {
        __fpurge(stdin);
        printf("How many partitions do you want your input to be split into? ");
    }
    while (scanf(" %ld", &splits) == 0);

    threadpool_t *pool POOL_CLEANUP = threadpool_create(splits);
    size_t chunk_size = ceil((double)n/splits);
    vals = calloc(n, sizeof *vals);
    struct accumulate_args *args = calloc(splits, sizeof *args);

    for (size_t i = 0 ; i < n ; ++i) {
        scanf(" %d", &vals[i]);
    }

    for (size_t i = 0, c = 0 ; i < n ; i += chunk_size, ++c) {
        args[c].vals = &vals[c];
        args[c].nvals = min(i + chunk_size, n) - i;
        threadpool_submit(pool, accumulate, &args[c]);
    }
    
    threadpool_finalize(pool, GRACEFUL_SHUTDOWN);

    uint64_t sum = 0;
    for (size_t i = 0 ; i < splits ; ++i) {
        sum += args[i].acc;
    }
    printf("Total sum: %ld\n", sum);

    free(args);
    free(vals);
    return 0;
}
```
This program take two inputs: `n` and `splits` for how many numbers to expect and how many threads to split the numbers into respectively.

The program will output the summed `n` inputs across all threads.

Possible output:
```sh
make clean run
...
./bin/out 
How many numbers do you want to add? 5
How many partitions do you want your input to be split into? 2
1
2
3
4
5
Total sum: 11
```