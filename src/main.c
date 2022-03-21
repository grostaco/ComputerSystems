#include <stdio.h>

#include "threadpool.h"

void print_sqrt(void* args) {
    printf("%f\n", sqrt(*(double*)args));
}

int main (void) {
    double d = 4.;
    threadpool_t *pool POOL_CLEANUP = threadpool_create(512);
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);
    threadpool_submit(pool, print_sqrt, &d);

    threadpool_finalize(pool);
    return 0;
}