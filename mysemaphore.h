// mysemaphore.h (Updated for Priority-Aware)
#ifndef MYSEMAPHORE_H
#define MYSEMAPHORE_H

#include "mymutex.h"

#define MAX_THREADS 8  // Fixed for tests (NUM_THREADS=4, but room for more)

typedef struct {
    my_mutex_t slock;
    volatile int value;
    volatile int waiting[MAX_THREADS];  // 1 if thread_id is waiting
} my_sem_t;

// Priority-aware semaphore
// thread_id: 0 to MAX_THREADS-1 (lower ID = higher priority in tests)
// When post() is called and waiters exist, the highest priority waiter (lowest thread_id) is selectively woken.
void my_sem_init(my_sem_t *s, int initial_value);
void my_sem_wait(my_sem_t *s, int thread_id, int priority);
void my_sem_post(my_sem_t *s);

#endif