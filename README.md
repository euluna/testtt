# Synchronization Primitives — Implementation Assignment

## Overview

In this assignment, you will implement five fundamental synchronization primitives from scratch in C using only **atomic operations** and **busy-waiting** (no `pthread_mutex`, `pthread_cond`, or `pthread_barrier` from the standard library). Each primitive builds on the previous one, giving you hands-on experience with how operating systems and threading libraries work under the hood.

A complete automated test suite (`test.c`) is provided. Your goal is to fill in the `// implement this` stubs so that **all tests pass**.

---

## Project Structure

| File | Role |
|---|---|
| `spinlock.h / spinlock.c` | **Ticket-based spinlock** — the foundational lock everything else is built on |
| `mymutex.h / mymutex.c` | **Mutex** — a simple wrapper around the spinlock |
| `mysemaphore.h / mysemaphore.c` | **Priority-aware counting semaphore** — supports bounded-buffer and priority wakeup |
| `mycondvar.h / mycondvar.c` | **Condition variable** — supports `wait`, `signal`, and `broadcast` |
| `mybarrier.h / mybarrier.c` | **Reusable barrier** — synchronizes N threads at a checkpoint (partially provided) |
| `test.c` | Automated test suite — **do not modify** |
| `Makefile` | Build instructions |
| `CMakeLists.txt` | CMake build configuration (alternative) |

---

## Build & Run

### Using Make (recommended)

```bash
make        # Compile
./test      # Run all tests
make clean  # Remove binary
```

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
./synchronizationProjectSolution_advanced
```

> **Requirements:** GCC with C11 support, pthreads, and a Linux/macOS environment (or WSL on Windows).

---

## What You Need to Implement

### 1. Ticket Spinlock (`spinlock.c`)

**Files:** `spinlock.h`, `spinlock.c`

A **fair (FIFO) ticket lock** that guarantees no starvation. The struct contains:

```c
typedef struct {
    volatile unsigned int next_ticket;
    volatile unsigned int now_serving;
} spinlock_t;
```

**Functions to implement:**

| Function | Description |
|---|---|
| `spinlock_init(spinlock_t *lock)` | Initialize both `next_ticket` and `now_serving` to 0. |
| `spinlock_acquire(spinlock_t *lock)` | Atomically fetch-and-increment `next_ticket` to get your ticket number. Spin (busy-wait) until `now_serving` equals your ticket. Use `sched_yield()` while spinning to be polite to the scheduler. |
| `spinlock_release(spinlock_t *lock)` | Increment `now_serving` to hand the lock to the next waiter. |

**Key concept:** Unlike a simple test-and-set spinlock, a ticket lock ensures threads acquire the lock in the order they requested it — just like a bakery counter.

**Hints:**
- Use `__sync_fetch_and_add(&lock->next_ticket, 1)` to atomically grab a ticket.
- Spin with `while (lock->now_serving != my_ticket) { sched_yield(); }`.

---

### 2. Mutex (`mymutex.c`)

**Files:** `mymutex.h`, `mymutex.c`

A thin wrapper around the spinlock:

```c
typedef struct {
    spinlock_t slock;
} my_mutex_t;
```

**Functions to implement:**

| Function | Description |
|---|---|
| `my_mutex_init(my_mutex_t *m)` | Initialize the internal spinlock. |
| `my_mutex_lock(my_mutex_t *m)` | Acquire the spinlock. |
| `my_mutex_unlock(my_mutex_t *m)` | Release the spinlock. |

This is intentionally simple — the complexity here lives in the spinlock.

---

### 3. Priority-Aware Semaphore (`mysemaphore.c`)

**Files:** `mysemaphore.h`, `mysemaphore.c`

A **counting semaphore** with priority-based wakeup. The struct contains:

```c
typedef struct {
    my_mutex_t slock;
    volatile int value;
    volatile int waiting[MAX_THREADS];  // 1 if thread_id is waiting
} my_sem_t;
```

**Functions to implement:**

| Function | Description |
|---|---|
| `my_sem_init(my_sem_t *s, int initial_value)` | Initialize the mutex, set `value`, and clear all `waiting` flags. |
| `my_sem_wait(my_sem_t *s, int thread_id, int priority)` | Decrement the semaphore. If `value <= 0`, mark `waiting[thread_id] = 1`, release the lock, spin until `waiting[thread_id]` is cleared by a `post()`, then return. |
| `my_sem_post(my_sem_t *s)` | Increment the semaphore. If any threads are waiting, find the one with the **highest priority (lowest `priority` value / lowest array index with `waiting[i] == 1`)** and clear its flag to wake it up. |

**Key concept:** When `my_sem_post` is called and multiple threads are blocked, the thread with the **highest priority** (as identified by the lowest numerical priority / thread index) must be woken first.

**Test breakdown:**
- **Basic test:** A bounded-buffer producer-consumer with 2 producers and 2 consumers, verifying no items are lost.
- **Priority test:** A low-priority thread (ID=1, priority=2) waits first, then a high-priority thread (ID=2, priority=1) waits. A single `post()` fires — the high-priority thread must win. A second `post()` releases the low-priority thread.

**Hints:**
- In `my_sem_wait`: lock → check value → if must wait, mark waiting, unlock, spin on `waiting[thread_id]`, then return.
- In `my_sem_post`: lock → increment value → scan `waiting[]` for the highest-priority waiter → clear its flag → unlock.
- Use `sched_yield()` in spin loops.

---

### 4. Condition Variable (`mycondvar.c`)

**Files:** `mycondvar.h`, `mycondvar.c`

A condition variable with slot-based tracking:

```c
typedef struct {
    my_mutex_t slock;
    volatile int waiting_flags[MAX_WAITERS];  // MAX_WAITERS = 64
} my_cond_t;
```

**Functions to implement:**

| Function | Description |
|---|---|
| `my_cond_init(my_cond_t *c)` | Initialize the mutex and clear all `waiting_flags`. |
| `my_cond_wait(my_cond_t *c, my_mutex_t *m, int thread_id)` | Mark `waiting_flags[thread_id] = 1`. **Release the external mutex `m`**. Spin until `waiting_flags[thread_id]` is cleared. **Re-acquire `m`** before returning. |
| `my_cond_signal(my_cond_t *c)` | Wake **exactly one** waiting thread by clearing its flag. |
| `my_cond_broadcast(my_cond_t *c)` | Wake **all** waiting threads by clearing all flags. |

**Key concept:** `my_cond_wait` must atomically release the mutex and begin waiting. The test verifies that `signal()` wakes exactly one thread — not zero, not two.

**Hints:**
- Use the internal `slock` to protect modifications to `waiting_flags`.
- In `wait`: lock the cond's internal lock → set flag → unlock internal lock → unlock external mutex → spin → re-lock external mutex.
- In `signal`: lock → find first set flag → clear it → unlock.

---

### 5. Reusable Barrier (`mybarrier.c`)

**Files:** `mybarrier.h`, `mybarrier.c`

A barrier that blocks until all N threads arrive, then releases them all. It is **reusable** — the same barrier can synchronize multiple phases.

```c
typedef struct {
    spinlock_t slock;
    int count;
    int total_threads;
    volatile int phaseNum;
} my_barrier_t;
```

`my_barrier_init` is **already provided**. You must implement:

| Function | Description |
|---|---|
| `my_barrier_wait(my_barrier_t *b)` | Acquire spinlock → increment `count` → if `count < total_threads`, capture current `phaseNum`, release spinlock, and spin until `phaseNum` changes. If `count == total_threads` (you are the last thread), reset `count` to 0, increment `phaseNum`, and release spinlock. |

**Key concept — reusability via phases:** A naïve barrier breaks if reused because fast threads can re-enter before slow threads leave. The `phaseNum` field solves this: waiting threads spin on their captured phase number, which only changes when the barrier is fully released. This cleanly separates consecutive barrier uses.

**Hints:**
- Capture `int local_phase = b->phaseNum;` before releasing the spinlock.
- Spin with `while (b->phaseNum == local_phase) { sched_yield(); }`.

---

## Test Suite Summary

The test suite in `test.c` runs five tests in sequence:

| # | Test | What It Validates |
|---|---|---|
| 1 | `test_mutex` | 4 threads increment a shared counter 100,000 times each. Final value must be exactly 400,000. |
| 2 | `test_semaphore_basic` | Bounded-buffer producer-consumer. Buffer must end at 0. |
| 3 | `test_semaphore_priority` | Priority-aware wakeup: high-priority thread must be woken before low-priority thread. |
| 4 | `test_condvar_singleWake` | `signal()` must wake exactly one of two waiting consumers. |
| 5 | `test_barrier` | 4 threads pass through a reusable barrier twice. Both phases must complete fully. |

All tests use `assert()`. A failing assertion will print the file, line, and condition that failed.

---



## Important Notes

- **Do not modify `test.c`** or any `.h` header file — only fill in the `.c` implementation files.
- **Do not use** `pthread_mutex_t`, `pthread_cond_t`, `sem_t`, or `pthread_barrier_t` from the standard library. The whole point is to build these yourself.
- All spinning should call `sched_yield()` to avoid burning CPU cycles needlessly.
- Use GCC built-in atomics (e.g., `__sync_fetch_and_add`) where needed for correctness.
- The code must compile cleanly with `-Wall -Wextra`.

---

## Expected Output (on success)

```
=== Synchronization Primitives Test Suite (with Personalized Requirements) ===

Testing Mutex (with fair ticket spinlock)...
Mutex test PASSED

Testing Basic Semaphore (bounded buffer)...
Producer thread ID: 0
Consumer thread ID: 1
Producer thread ID: 2
Consumer thread ID: 3
Basic semaphore test PASSED

Testing Priority-Aware Semaphore...
   Requirement: sem_post() must wake the HIGHEST priority waiting thread (lowest thread_id).
   Test scenario: Low priority (ID=2) waits first, high priority (ID=1) waits second.
   One post() → high priority must acquire (acquired_by == 1).
1posts=[0, 0], count=0
2posts=[0, 0], count=0
Priority-aware test PASSED (high priority won first resource)

Testing Condition Variable for single wake...
Signalling once...
Signal precision verified: Only 1 consumer woke up.
Condition Variable test PASSED

Testing Barrier...
barrier_test: phase1
barrier_test: phase1
barrier_test: phase1
barrier_test: phase1
barrier_test: phase2
barrier_test: phase2
barrier_test: phase2
barrier_test: phase2
Barrier test PASSED

All tests PASSED! Implementation correct.
```

Good luck!
