#pragma once 

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#define COLLECTOR_CLEANUP __attribute__((cleanup(generic_cleanup)))

static inline void generic_cleanup(void *x) {
    free(*(void**)x);
}

/**
 * @struct collector
 * @brief Structure representing a collector process

 */
struct collector {
    pid_t process_id; // Collector's process ID
    int readfd;       // Descriptor for the collector to read from
    int writefd;      // Descriptor to write to the collector
    int outfd;        // Descriptor to read from the collector
};

/**
 * @brief Structure representing the result of collectors
 * 
 */
struct collector_result {
    pid_t process_id; // Collector's process ID
    uint64_t sum;     // Summed value of all collector's input
};

/**
 * @brief Create a new collector
 * 
 * @param expected required pushed elements until sum
 * @return struct collector 
 */
struct collector collector_create(size_t expected);

/**
 * @brief Push a value to be summed to a collector
 * 
 * @param collector collector for the value to pushed onto
 * @param val value to be pushed onto the collector
 */
void collector_push(struct collector *collector, uint32_t val);

/**
 * @brief Collect summed results from collectors
 * 
 * @param collectors collectors to be collected from 
 * @param n collector count
 * @return collected results 
 */
struct collector_result *collector_collect(struct collector* collectors, size_t n);