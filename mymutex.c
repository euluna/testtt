// mymutex.c (Reference Solution – Simple wrapper)
#include "mymutex.h"

void my_mutex_init(my_mutex_t *m)
{
    // implement this
    spinlock_init(&m->slock);
}

void my_mutex_lock(my_mutex_t *m)
{
    // implement this
    spinlock_acquire(&m->slock);
}

void my_mutex_unlock(my_mutex_t *m)
{
    // implement this
    spinlock_release(&m->slock);
}