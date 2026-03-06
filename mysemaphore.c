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
    my_mutex_lock(&s->slock);

    s->value--;

    if (s->value < 0)
    {
        // Store the priority instead of just '1'
        // We assume priority > 0 (e.g., 1 is high, 2 is lower)
        s->waiting[thread_id] = priority;

        my_mutex_unlock(&s->slock);

        // Spin until the flag becomes 0
        while (s->waiting[thread_id] != 0)
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
    my_mutex_lock(&s->slock);
    s->value++;

    if (s->value <= 0)
    {
        int best_thread = -1;
        int highest_priority_val = 2147483647; // Max INT

        // Look at everyone currently waiting
        for (int i = 0; i < MAX_THREADS; i++)
        {
            // If the value is > 0, they are waiting with that priority
            if (s->waiting[i] > 0)
            {
                if (s->waiting[i] < highest_priority_val)
                {
                    highest_priority_val = s->waiting[i];
                    best_thread = i;
                }
            }
        }

        // Release the one with the smallest priority number (highest priority)
        if (best_thread != -1)
        {
            s->waiting[best_thread] = 0;
        }
    }
    my_mutex_unlock(&s->slock);
}
/*
void my_sem_wait(my_sem_t *s, int thread_id, int priority)
{
    // implement this
    my_mutex_lock(&s->slock);
    s->waiting[thread_id] = 1;
    s->value--;
    if (s->value < 0)
    {

        my_mutex_unlock(&s->slock);
        while (s->waiting[thread_id] == 1)
        {
            sched_yield();
        }
    }
    else
    {
        s->waiting[thread_id] = 0;
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
}*/