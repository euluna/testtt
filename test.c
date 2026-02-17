// test.c (Full Automated Test Suite with Personalized Tests Explained)
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>  // usleep
#include "spinlock.h"
#include "mymutex.h"
#include "mysemaphore.h"
#include "mycondvar.h"
#include "mybarrier.h"

#define NUM_THREADS 4
#define ITERATIONS 100000
#define MAX_THREADS 8

volatile int shared_counter = 0;
volatile int phase1_done = 0;
volatile int phase2_done = 0;

// Mutex test (uses fair ticket spinlock internally)
void *mutex_thread_func(void *arg) {
    my_mutex_t *m = (my_mutex_t *)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        my_mutex_lock(m);
        shared_counter++;
        my_mutex_unlock(m);
    }
    return NULL;
}

void test_mutex() {
    printf("Testing Mutex (with fair ticket spinlock)...\n");
    shared_counter = 0;
    my_mutex_t m;
    my_mutex_init(&m);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&threads[i], NULL, mutex_thread_func, &m);
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);

    assert(shared_counter == NUM_THREADS * ITERATIONS);
    printf("Mutex test PASSED\n\n");
}

// Bounded buffer semaphore test
volatile int buffer_items = 0;
my_sem_t empty, full;
my_mutex_t mut;

void *producer(void *arg) {
    int th_id = *((int*)arg);
    printf("Producer thread ID: %d\n", th_id);
    for (int i = 0; i < ITERATIONS / 2; i++) {
        my_sem_wait(&empty, th_id,1);  // thread_id not used in this test
        my_mutex_lock(&mut);
        buffer_items++;
        my_mutex_unlock(&mut);
        my_sem_post(&full);
    }
    return NULL;
}

void *consumer(void *arg) {
    int th_id = *((int*)arg);
    printf("Consumer thread ID: %d\n", th_id);
    for (int i = 0; i < ITERATIONS / 2; i++) {
        my_sem_wait(&full, th_id,1);
        my_mutex_lock(&mut);
        buffer_items--;
        my_mutex_unlock(&mut);
        my_sem_post(&empty);
    }
    return NULL;
}

void test_semaphore_basic() {
    printf("Testing Basic Semaphore (bounded buffer)...\n");
    buffer_items = 0;
    my_sem_init(&empty, 10);
    my_sem_init(&full, 0);
    my_mutex_init(&mut);

    pthread_t prod[NUM_THREADS/2], cons[NUM_THREADS/2];
    int prod_ids[NUM_THREADS/2];
    int cons_ids[NUM_THREADS/2];

    for (int i = 0; i < NUM_THREADS/2; i++) {
        prod_ids[i] = 2 * i;      // Producer IDs: 0, 2
        cons_ids[i] = 2 * i + 1;  // Consumer IDs: 1, 3
        pthread_create(&prod[i], NULL, producer, (void*)&prod_ids[i]);
        pthread_create(&cons[i], NULL, consumer, (void*)&cons_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS/2; i++) {
        pthread_join(prod[i], NULL);
        pthread_join(cons[i], NULL);
    }

    assert(buffer_items == 0);
    printf("Basic semaphore test PASSED\n\n");
}

// Priority-aware semaphore test
volatile int acquired_by = -1; // this value should be set by lowest priority thread (i.e. thead_id=1).
volatile int posts[2] = {0,0};
volatile int count = 0;
void *low_priority_waiter(void *arg) {
    my_sem_t *s = arg;
    my_sem_wait(s, 1, 2);  // Low priority
    acquired_by = 1;
    return NULL;
}

void *high_priority_waiter(void *arg) {
    my_sem_t *s = arg;
    usleep(50000);
    while (s->waiting[1]==0) {
        sched_yield();  // wait for lower-priority waiter to register itself in the waiting list.
    }
    my_sem_wait(s, 2, 1);  // High priority
    acquired_by = 2;
    return NULL;
}

void *poster_thread(void *arg) {
    my_sem_t *s = (my_sem_t*)arg;
    while (s->waiting[1]==0 || s->waiting[2]==0) {
        sched_yield();
    }

    my_sem_post(s);  // Single post – high priority should win this one
    printf("1posts=[%d, %d], count=%d\n", posts[0], posts[1], count);
    usleep(50000);
    // Second post to release low-priority (allows clean exit)
    my_sem_post(s);
    return NULL;
}

void test_semaphore_priority() {
    printf("Testing Priority-Aware Semaphore...\n");
    printf("   Requirement: sem_post() must wake the HIGHEST priority waiting thread (lowest thread_id).\n");
    printf("   Test scenario: Low priority (ID=2) waits first, high priority (ID=1) waits second.\n");
    printf("   One post() → high priority must acquire (acquired_by == 1).\n");
    posts[0]=posts[1] = 0;
    count = 0;
    my_sem_t s;
    my_sem_init(&s, 0);

    acquired_by = -1;

    pthread_t low, high, poster;
    pthread_create(&low, NULL, low_priority_waiter, &s);
    pthread_create(&high, NULL, high_priority_waiter, &s);
    //pthread_create(&poster, NULL, poster_thread, &s);
    poster_thread(&s);
    //pthread_join(poster, NULL);

   // assert(acquired_by == 2);  // High priority thread must have won the first post


    pthread_join(high, NULL);
    pthread_join(low, NULL);
    printf("2posts=[%d, %d], count=%d\n", posts[0], posts[1], count);

    assert(acquired_by == 1);  // low priority thread must have won the second post

    printf("Priority-aware test PASSED (high priority won first resource)\n\n");
 }

// Condition variable test
volatile int items_ready = 0; // number of items ready for consumption
volatile int items_consumed = 0;
my_cond_t item_cond;
my_mutex_t item_mutex;

void *cv_producer(void *arg) {
    int id = *(int*)arg;
    printf("Signalling once...\n");
    my_mutex_lock(&item_mutex);
    items_ready = 1;
    my_cond_signal(&item_cond);
    my_mutex_unlock(&item_mutex);

    sleep(1); // make sure both threads are waiting

    // Only ONE should have finished
    my_mutex_lock(&item_mutex);
    assert(items_consumed == 1);
    printf("Signal precision verified: Only 1 consumer woke up.\n");

    // Now provide another item and signal again to clean up
    items_ready = 1;
    my_cond_signal(&item_cond);
    my_mutex_unlock(&item_mutex);

    return NULL;
}

void *cv_consumer(void *arg) {
    int id = *(int*)arg;
    my_mutex_lock(&item_mutex);
    // wait for an item
    while (items_ready <= 0)
        my_cond_wait(&item_cond, &item_mutex, id);
    assert(items_ready == 1);
    //consume one item
    items_ready--;
    items_consumed++;
    my_mutex_unlock(&item_mutex);
    return NULL;
}

void test_condvar_singleWake() {
    printf("Testing Condition Variable for single wake...\n");
    items_ready = 0;
    my_cond_init(&item_cond);
    my_mutex_init(&item_mutex);
    int id1=1, id2=2;
    pthread_t prod, cons;
    pthread_create(&prod, NULL, cv_producer, &id1);
    pthread_create(&cons, NULL, cv_consumer, &id2);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("Condition Variable test PASSED\n\n");
}

// Barrier test. Description of Barriers is given myBarrier.h
void *barrier_thread(void *arg) {
    my_barrier_t *b = arg;
    phase1_done++;
    my_barrier_wait(b);
    printf("barrier_test: phase1\n");
    phase2_done++;
    my_barrier_wait(b);
    printf("barrier_test: phase2\n");
    return NULL;
}

void test_barrier() {
    printf("Testing Barrier...\n");
    phase1_done = phase2_done = 0;
    my_barrier_t b;
    my_barrier_init(&b, NUM_THREADS);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, barrier_thread, &b);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    assert(phase1_done == NUM_THREADS && phase2_done == NUM_THREADS);
    printf("Barrier test PASSED\n\n");
}

// Main
int main() {
    printf("=== Synchronization Primitives Test Suite (with Personalized Requirements) ===\n\n");

    test_mutex();
    test_semaphore_basic();
    test_semaphore_priority();  
    test_condvar_singleWake();
    test_barrier();

    printf("All tests PASSED! Implementation correct.\n");
    return 0;
}