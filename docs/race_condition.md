# Race Condition

```c
/*
 * To compile: gcc 00_race_condition.c -o 00_race_condition.o -lpthread
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define NITER 1000000

int cnt = 0;
pthread_mutex_t lock;

void * Count(void __attribute__((unused)) *a)
{
    int i, tmp;
    pthread_mutex_lock(&lock);
    for(i = 0; i < NITER; i++)
    {
        tmp = cnt;      /* copy the global cnt locally */
        tmp = tmp+1;    /* increment the local copy */
        cnt = tmp;      /* store the local value into the global cnt */ 
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void)
{
    pthread_mutex_init(&lock, NULL);
    pthread_t tid1, tid2;

    if(pthread_create(&tid1, NULL, Count, NULL))
    {
      printf("\n ERROR creating thread 1");
      exit(1);
    }

    if(pthread_create(&tid2, NULL, Count, NULL))
    {
      printf("\n ERROR creating thread 2");
      exit(1);
    }

    if(pthread_join(tid1, NULL))	/* wait for the thread 1 to finish */
    {
      printf("\n ERROR joining thread");
      exit(1);
    }

    if(pthread_join(tid2, NULL))        /* wait for the thread 2 to finish */
    {
      printf("\n ERROR joining thread");
      exit(1);
    }

    if (cnt < 2 * NITER) 
        printf("\n BOOM! cnt is [%d], should be %d\n", cnt, 2*NITER);
    else
        printf("\n OK! cnt is [%d]\n", cnt);
  
    pthread_mutex_destroy(&lock);
    pthread_exit(NULL);
}
```

- Added a mutex guard on the critical section between line 18-20
    - Although a mutex could've been added inside of the loop, the overhead of the mutex cannot justify a lock over a simple operation.
    - As such adding a lock outside will be more performant (benchmarking below)

With mutex lock in the for loop using `NITER=100`
```
$ time ./bin/out

 OK! cnt is [200000000]

real    0m13.877s
user    0m18.919s
sys     0m8.729s
```

With mutex lock outside the for loop using `NITER=1000000`
```
$ time ./bin/out

 OK! cnt is [2000000]

real    0m0.001s
user    0m0.001s
sys     0m0.000s
```
