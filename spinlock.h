// spinlock.h (Fair Ticket Lock – Personalized Requirement #1)
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <sched.h>  // for sched_yield()

typedef struct {
    volatile unsigned int next_ticket;
    volatile unsigned int now_serving;
} spinlock_t;

// Ticket lock provides FAIRNESS (FIFO order) – no starvation under contention.
// Each thread gets a ticket and waits its turn. This satisfies personalized requirement #1.
void spinlock_init(spinlock_t *lock);
void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);

#endif