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