// mycondvar.h
#ifndef MYCONDVAR_H
#define MYCONDVAR_H

#include "mymutex.h"

#define MAX_WAITERS 64

typedef struct {
    my_mutex_t slock;
    // Track which thread slots are actually waiting
    volatile int waiting_flags[MAX_WAITERS];
} my_cond_t;

void my_cond_init(my_cond_t *c);
void my_cond_wait(my_cond_t *c, my_mutex_t *m, int thread_id);
void my_cond_signal(my_cond_t *c);
void my_cond_broadcast(my_cond_t *c);

#endif