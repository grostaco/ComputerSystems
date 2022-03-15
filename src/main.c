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
