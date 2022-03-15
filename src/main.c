#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "collector.h"

int main (int argc, char **argv) {
    // Create a program that has an invocation ./program <splits> which splits the input into <splits> collectors.
    // Ensure that splits was passed in
    if (argc != 2) {
        fprintf(stderr, "Invalid usage. Use %s <splits>\n", argv[0]);
        return 1;
    }

    // If splits is passed in, ensure that it is a non-negative integer
    char *endptr;
    long splits = strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr) {
        fprintf(stderr, "Splits must be an integer\n");
        return 1;
    }

    if (splits <= 0) {
        fprintf(stderr, "Splits must be greater than 0\n");
        return 1;
    }

    // Create <split> amount of collectors
    struct collector *collectors COLLECTOR_CLEANUP = calloc(splits, sizeof *collectors);
    uint32_t _values[256];
    uint32_t *values = _values; 
    size_t collected = 0;

    // Read integers from stdin
    char buf[64];
    while (fgets(buf, 64, stdin)) {
        values[collected++] = strtol(buf, &endptr, 10);
    }

    // Start splitting the input into collectors
    size_t chunk = collected / splits;
    for (int i = 0 ; i < splits-1 ; ++i) {
        collectors[i] = collector_create(chunk);

        for (size_t j = 0 ; j < chunk ; ++j) {
            collector_push(&collectors[i], *values++);
        }

        collected -= chunk;
    } 

    collectors[splits-1] = collector_create(collected);
    for (size_t i = 0 ; i < collected ; ++i) {
        collector_push(&collectors[splits - 1], *values++);
    }

    // Poll the results
    struct collector_result *results COLLECTOR_CLEANUP = collector_collect(collectors, splits);

    // Add the results up
    uint64_t total_sum = 0;
    for (int i = 0 ; i < splits ; ++i) {
        total_sum += results[i].sum;
    }

    // Print the final result
    printf("Accumulated sum: %ld\n", total_sum);

}