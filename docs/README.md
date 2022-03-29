# Docs

This directory contains resources explaining files inside of this project.

# ToC
- [Library-ish things](#library-ish-things)
    - [Collector](#collectorcollectormd)
    - [Thread Pool](#thread-poolthreadpoolmd)
    - [Dining](#diningdiningmd)
- Assignments
    - 

# Library-ish things
## [Collector](collector.md)
A program using `fork(2)` to sum multiple numbers using `pipe(2)` for IPC. See [here](collector.md) for more information.

## [Thread pool](threadpool.md)
A program implementing a thread pool using `pthreads(7)` to sum multiple numbers. See [here](threadpool.md) for more information. 

## [Dining](dining.md)
A program implementing Dijkstra's solution to the dining philosophers problem using semaphore (see `sem_overview(7)`). See [here](dining.md) for more information.

# Assignments
## [Race condition](race_condition.md)
A solution to assignment 10's first question to resolve a race condition.

## [OpenMP accumulator](openmp_acc.md)
A solution to assignment 10's second question to rewrite assignment 8 using OpenMP.
