// mycondvar.c
#include "mycondvar.h"

void my_cond_init(my_cond_t *c)
{
    // implement this
    my_mutex_init(&c->slock);
    for (int i = 0; i < MAX_WAITERS; i++)
    {
        c->waiting_flags[i] = 0;
    }
}

void my_cond_wait(my_cond_t *c, my_mutex_t *m, int thread_id)
{
    // implement this
    my_mutex_lock(&c->slock);
    c->waiting_flags[thread_id] = 1;
    my_mutex_unlock(&c->slock);

    my_mutex_unlock(m);
    while (c->waiting_flags[thread_id] == 1)
    {
        sched_yield();
    }
    my_mutex_lock(m);
}

void my_cond_signal(my_cond_t *c)
{
    // implement this
    my_mutex_lock(&c->slock);
    for (int i = 0; i < MAX_WAITERS; i++)
    {
        if (c->waiting_flags[i] == 1)
        {
            c->waiting_flags[i] = 0;
            break;
        }
    }
    my_mutex_unlock(&c->slock);
}

void my_cond_broadcast(my_cond_t *c)
{
    // implement this
    my_mutex_lock(&c->slock);
    for (int i = 0; i < MAX_WAITERS; i++)
    {
        c->waiting_flags[i] = 0;
    }
    my_mutex_unlock(&c->slock);
}