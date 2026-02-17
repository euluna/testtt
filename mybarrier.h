// mybarrier.h
#ifndef MYBARRIER_H
#define MYBARRIER_H

#include "spinlock.h"
/*
 *A Barrier is a synchronization primitive used to ensure that a group of threads all reach a certain point of
 *execution before any of them are allowed to proceed further. Think of it like a group hike: even if some hikers
 *are faster than others, the group agrees to wait at a specific "checkpoint." No one starts the next leg of the
 *trail until the last person arrives at the checkpoint.
* 1-count below represents the checkpoint above. Each thread that enters my_barrier_wait increments b->count
*   under a spinlock. This acts as the counter for "hikers who have arrived at the checkpoint."
* 2- If a thread arrives and b->count < total_threads, it captures the current phaseNum and enters a spin-loop.
* It releases the spinlock while waiting (sched_yield) so other threads can enter the barrier.
* 3- If a thread arrives and b->count < total_threads, it captures the current phaseNum and enters a spin-loop.
*   It releases the spinlock while waiting (sched_yield) so other threads can enter the barrier.
* 4- When the last thread arrives it must resent the count for the next phase, increment phaseNum and return. This
*   should automatically release all the waiting threads now.

 */
typedef struct {
    spinlock_t slock;
    int count;
    int total_threads;
    volatile int phaseNum;  // For reusable barrier
} my_barrier_t;

// Reusable barrier for N threads
void my_barrier_init(my_barrier_t *b, int n);
void my_barrier_wait(my_barrier_t *b);

#endif