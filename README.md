# Fork Collector 

A collector for adding multiple numbers via `fork`. Created for a university class.

# ToC
- [API](#api)
    - [Collector structs](#collector-structs)
    - [Collector functions](#collector-functions)
- [Implementation](#implementation)
    - [Collectors](#collectors)
- [Examples](#examples)
    - [Collector to parallelize sum]()
- [Disclaimer](#disclaimer)

# API
## Collector structs
```c
struct collector {
    pid_t process_id;
    int readfd;
    int writefd;
    int outfd;
};
```
| field | description |
|-------|------------|
| process_id | The process ID of the collector |
| readfd     | The file descriptor to read from the collector |
| writefd | The file descriptor to write to the collector |
| outfd   | The file descriptor to read the final output from the collector |

```c
struct collector_result {
    pid_t process_id;
    uint64_t sum;
};
```
| field | description |
|-------|-------------|
| process_id | The process ID of the collector |
| sum        | The sum of all the values given to this collector |

## Collector functions
```c
struct collector collector_create(size_t expected)
```
Creates a collector which takes `expected` values until exiting and writing the final value to `outfd`.

```c
void collector_push(struct collector *collector, uint32_t val)
```
Push `val` onto the pointed collector `collector` to be processed.

```c
struct collector_result *collector_collect(struct collector* collectors, size_t n)
```
Return the value from `outfd` wrapped in `struct collector_result` of all `n` collectors from `collectors`.

# Implementation
![](static/graphviz.svg)
Figure of collector relationships with the parent process.
## Collectors

### Creation
Collectors are the main idea behind this implementation. The main process can delegate computation tasks to multiple collectors by calling `collector_create`. These collectors will then collect new input from the parent until a limit is exceeded. New collectors are spawned via `fork`.

### Collection
The collector function `collector_collect` use `poll(2)` to all `outfd` in the `collectors` passed to the function to wait until collectors finish writing their values to `outfd`. 

# Examples
## Single collector
```c
#include <stdio.h>
#include "collector.h"

int main (void) {
    // create a collector that add 4 numbers
    struct collector collector = collector_create(4); 

    // Push 4 numbers to be added
    collector_push(&collector, 1);
    collector_push(&collector, 2);
    collector_push(&collector, 3);
    collector_push(&collector, 4);

    // ... the parent process is free to do anything here ...

    // poll for the result 
    struct collector_result *results = collector_collect(&collector, 1);

    // print the output
    printf("%ld\n", results[0].sum);
}
```
Output:
```
$ make clean run
...
./bin/out
10
```

## Multiple collectors
```c
#include <stdio.h>
#include "collector.h"

int main (void) {
    // create a collector that add 4 numbers
    struct collector collector = collector_create(4); 

    // Push 4 numbers to be added
    collector_push(&collector, 1);
    collector_push(&collector, 2);
    collector_push(&collector, 3);
    collector_push(&collector, 4);

    // ... the parent process is free to do anything here ...

    // poll for the result 
    struct collector_result *results = collector_collect(&collector, 1);

    // print the output
    printf("%ld\n", results[0].sum);
}
```

## Advanced