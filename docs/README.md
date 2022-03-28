# Docs

This directory contains resources explaining files inside of this project.

# ToC
- [Collector](#collector)
- [Thread Pool](#thread-pool)
- [Dining](#dining)

# Collector
A program using `fork(2)` to sum multiple numbers using `pipe(2)` for IPC. See [here](collector.md) for more information.

# Thread pool
A program implementing a thread pool using `pthreads(7)` to sum multiple numbers. See [here](threadpool.md) for more information. 

# Dining
A program implementing Dijkstra's solution to the dining philosophers problem using semaphore (see `sem_overview(7)`). See [here](dining.md) for more information.
