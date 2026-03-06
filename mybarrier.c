// mybarrier.c
#include "mybarrier.h"

void my_barrier_init(my_barrier_t *b, int n)
{
    spinlock_init(&b->slock);
    b->total_threads = n;
    b->count = 0;
    b->phaseNum = 0;
}
// This function is basically called when thread has done its phase and now wait for others to reach.
void my_barrier_wait(my_barrier_t *b)
{
    // implement this.
    spinlock_acquire(&b->slock);
    b->count++;
    int local_phase = b->phaseNum;
    if (b->count < b->total_threads)
    {
        spinlock_release(&b->slock);
        while (b->phaseNum == local_phase)
        {
            sched_yield();
        }
    }
    else if (b->count == b->total_threads)
    {
        b->count = 0;
        b->phaseNum++;
        spinlock_release(&b->slock);
    }
}