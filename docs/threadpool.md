# Thread Pool
A simple thread pool implementation inspired by [om.bitsian's implementation](https://programmer.group/c-simple-thread-pool-based-on-pthread-implementation.html).

# ToC
- [API](#api)
    - [Thread pool structs](#thread-pool-structs)
    - [Thread pool functions](#thread-pool-functions)
- [Implementation](#implementation)
    - [Collectors](#collectors)
        - [Creation](#creation)
        - [Collection](#collection)
- [Examples](#examples)
    - [Single collector](#single-collector)
    - [Multiple collectors](#multiple-collectors)
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
void threadpool_finalize(threadpool_t *pool);
```
Wait until every task is finished then join every threads in the pool.

```c
void threadpool_cleanup(void *vpool);
```
Helper function to free pool resources. Should never be called directly; instead use `POOL_CLEANUP` macro.

```c
void *threadpool_thread(void* vpool);
```
Helper function to take new tasks. Should never be called directly.