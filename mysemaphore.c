// mysemaphore.c
#include "mysemaphore.h"

void my_sem_init(my_sem_t *s, int initial_value)
{
    // implement this
    my_mutex_init(&s->slock);
    s->value = initial_value;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        s->waiting[i] = 0;
    }
}

void my_sem_wait(my_sem_t *s, int thread_id, int priority)
{
    // implement this
    my_mutex_lock(&s->slock);
    s->value--;
    if (s->value < 0)
    {
        s->waiting[thread_id] = 1;
        my_mutex_unlock(&s->slock);
        while (s->waiting[thread_id] == 1)
        {
            sched_yield();
        }
    }
    else
    {
        my_mutex_unlock(&s->slock);
    }
}

void my_sem_post(my_sem_t *s)
{
    // implement this
    my_mutex_lock(&s->slock);
    s->value++;
    if (s->value <= 0)
    { // means hv threads waiting
        for (int i = 0; i < MAX_THREADS; i++)
        {
            if (s->waiting[i] == 1)
            {
                s->waiting[i] = 0;
                break;
            }
        }
    }
    my_mutex_unlock(&s->slock);
}