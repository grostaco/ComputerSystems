#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define N 16

int main () {
    int rd = open("/dev/urandom", O_RDONLY);

    uint16_t *numbers = calloc(N, sizeof *numbers);
    uint64_t sum = 0;
    read(rd, numbers, N * sizeof *numbers);

#pragma omp parallel for 
    for (size_t i = 0 ; i < N ; ++i) {
        // printf is usually locked.
        printf("Got number: %d\n", numbers[i]);
        sum += numbers[i];
    }

    printf("Accumulated sum: %ld\n", sum);
}