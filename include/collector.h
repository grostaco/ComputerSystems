#pragma once 

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#define RESULTS_CLEANUP __attribute__((cleanup(generic_cleanup)))

static inline void generic_cleanup(void *x) {
    free(*(void**)x);
}

struct collector {
    pid_t process_id;
    int readfd;
    int writefd;
    int outfd;
};

struct collector_result {
    pid_t process_id;
    uint64_t sum;
};

struct collector collector_create(size_t expected);
void collector_push(struct collector *collector, uint32_t val);
struct collector_result *collector_collect(struct collector* collectors, size_t n);