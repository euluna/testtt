// spinlock.c (Reference Solution – Ticket Lock)
#include "spinlock.h"

void spinlock_init(spinlock_t *lock)
{
    // implement this
    lock->next_ticket = 0;
    lock->now_serving = 0;
}

void spinlock_acquire(spinlock_t *lock)
{
    unsigned int my_ticket = __sync_fetch_and_add(&lock->next_ticket, 1);
    // implement this
    while (lock->now_serving != my_ticket)
    {
        sched_yield();
    }
}

void spinlock_release(spinlock_t *lock)
{
    // implement this
    lock->now_serving++;
}