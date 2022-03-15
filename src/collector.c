#include "collector.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>

/**
 * @brief Create a new collector
 * 
 * @param expected required pushed elements until sum
 * @return struct collector 
 */
struct collector collector_create(size_t expected) {
    // Create a pipe for parent <-> child communication
    int pipefd[2];
    pipe(pipefd);

    // Extra pipe for collector_collect. The write end is disregarded.
    int outfd[2];
    pipe(outfd);

    // Begin forking
    pid_t pid = fork();
    
    if (pid > 0) {
        // Return back to parent with the collector descriptor
        return (struct collector){.process_id = pid, .readfd = pipefd[0], .writefd = pipefd[1], .outfd = outfd[0]};
    }

    // Begin receiving values and summing
    uint64_t sum = 0;
    uint32_t num;

    while (expected > 0) {
        read(pipefd[0], &num, sizeof num);
        sum += num;
        --expected;
    }

    // Once the expected quota is reached, write back to outfd 
    write(outfd[1], &sum, sizeof sum);

    // Close the write end.
    close(outfd[1]);
    exit(0);
}

void collector_push(struct collector *collector, uint32_t val) {
    // Nothing special. A nice wrapper to write to the end of the collector pipe.
    write(collector->writefd, &val, sizeof val);
}

// Consume every collectors and close their connections
struct collector_result *collector_collect(struct collector* collectors, size_t n) {
    // Allocate enough memory to contain every collected results
    struct collector_result *results = malloc(sizeof *results * n);
    struct pollfd *pfds = calloc(n, sizeof *pfds);
    int nfds = n;

    // Watch for events when outfd is ready
    for (int i = 0 ; i < nfds ; ++i) {
        pfds[i].fd = collectors[i].outfd;
        pfds[i].events = POLLIN;
    }

    // While there is a collector that's not collected
    while (n) {
        int fd;

        // Poll until a collector is ready
        fd = poll(pfds, nfds, -1);
        if (fd == -1) {
            perror("poll");
        }

        // Run through the list to find an available outfd to read from 
        for (int i = 0 ; i < nfds ; ++i) {
            if (pfds[i].revents & POLLIN) {
                uint64_t sum;

                read(pfds[i].fd, &sum, sizeof sum);
                results[nfds - n] = (struct collector_result){.process_id = collectors[i].process_id, .sum=sum};
                close(pfds[i].fd);
                n--;
            }
        }
    }

    return results;
}
