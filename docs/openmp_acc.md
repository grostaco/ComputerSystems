# OpenMP Accumulator

```c
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
```

The program will read `N` random numbers from `/dev/urandom` then add them up through OpenMP's `loop` construct. After everything has been added, the program will print out all the accumulated read values.

Possible output:
```sh
$ make clean run
...
./bin/out 
Got number: 39424
Got number: 29160
Got number: 14283
Got number: 1952
Got number: 62085
Got number: 40454
Got number: 49583
Got number: 59401
Got number: 54781
Got number: 12487
Got number: 13307
Got number: 17932
Got number: 44972
Got number: 19086
Got number: 13907
Got number: 28462
Accumulated sum: 501276
```