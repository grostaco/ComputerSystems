#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>

#define NPHILOSOPHERS 8
#define THINKING 0
#define HUNGRY 1
#define EATING 2


#define BIT(x, i) (((x) & (i)) == (i))
#define LWRAP(i, N) (((i) + (N) - 1) % (N))
#define RWRAP(i, N) (((i) + 1) % (N))

typedef struct philosopher {
    uint32_t state;
    sem_t semaphore;
}philosopher_t;

typedef struct dining_table {
    philosopher_t *philosophers;
    pthread_mutex_t lock;
    size_t nphilosophers;
}dining_table_t;

dining_table_t *dining_create(size_t nphilosophers);
void dining_destroy(dining_table_t *table);
void dining_inquire(dining_table_t *table, size_t philosopher);
void dining_acquire_fork(dining_table_t *table, size_t philosopher);
void dining_relinquish_fork(dining_table_t *table, size_t philosopher);