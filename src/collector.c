#include "collector.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>

struct collector collector_create(size_t expected) {
    int pipefd[2];
    pipe(pipefd);
    int outfd[2];
    pipe(outfd);

    pid_t pid = fork();
    
    if (pid > 0) {
        return (struct collector){.process_id = pid, .readfd = pipefd[0], .writefd = pipefd[1], .outfd = outfd[0]};
    }

    uint64_t sum = 0;
    uint32_t num;

    while (expected > 0) {
        read(pipefd[0], &num, sizeof num);
        sum += num;
        --expected;
    }

    write(outfd[1], &sum, sizeof sum);
    close(outfd[1]);
    exit(0);
}

void collector_push(struct collector *collector, uint32_t val) {
    write(collector->writefd, &val, sizeof val);
}

// Consume every collectors and close their connections
struct collector_result *collector_collect(struct collector* collectors, size_t n) {
    struct collector_result *results = malloc(sizeof *results * n);
    struct pollfd *pfds = calloc(n, sizeof *pfds);
    int nfds = n;

    for (int i = 0 ; i < nfds ; ++i) {
        pfds[i].fd = collectors[i].outfd;
        pfds[i].events = POLLIN;
    }

    while (n) {
        int fd;

        fd = poll(pfds, nfds, -1);
        if (fd == -1) {
            perror("poll");
        }

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
