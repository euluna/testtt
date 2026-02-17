// mymutex.h
#ifndef MYMUTEX_H
#define MYMUTEX_H

#include "spinlock.h"

typedef struct {
    spinlock_t slock;
} my_mutex_t;

// Simple mutex using spinlock (mutual exclusion only)
void my_mutex_init(my_mutex_t *m);
void my_mutex_lock(my_mutex_t *m);
void my_mutex_unlock(my_mutex_t *m);

#endif
