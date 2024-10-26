#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* Include necessary header files */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

typedef uint32_t worker_t;

/* Thread states */
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define EXITED 3

/* Priority definitions */
#define NUMPRIO 4
#define HIGH_PRIO 3
#define MEDIUM_PRIO 2
#define DEFAULT_PRIO 1
#define LOW_PRIO 0

/* Thread Control Block */
typedef struct TCB {
    worker_t thread_id;          // Thread ID
    int status;                  // Thread status
    ucontext_t context;          // Thread context
    void* stack;                 // Stack pointer
    int priority;                // Thread priority
    void* retval;                // Return value
    struct TCB* next;            // Pointer to next TCB in queue
    struct TCB* all_next;        // Pointer to next TCB in all threads list
    struct timeval creation_time;        // Time when thread was created
    struct timeval first_scheduled_time; // Time when thread was first scheduled
    struct timeval end_time;             // Time when thread exited
    int first_scheduled;         // Flag to indicate if first scheduled
    long elapsed_time;           // Total execution time
} tcb;

/* Mutex struct definition */
typedef struct worker_mutex_t {
    int locked;                  // Mutex lock flag
    tcb* owner;                  // Owner of the mutex
    tcb* wait_queue;             // Queue of threads waiting for the mutex
} worker_mutex_t;

/* Runqueue definition */
typedef struct runqueue_t {
    tcb* head;
    tcb* tail;
} runqueue_t;

/* Multi-level feedback queue */
#define MAX_LEVELS NUMPRIO

typedef struct mlfq_t {
    runqueue_t queues[MAX_LEVELS];
} mlfq_t;

/* Function Declarations */

/* Create a new thread */
int worker_create(worker_t* thread, pthread_attr_t* attr, void* (*function)(void*), void* arg);

/* Yield CPU possession voluntarily */
int worker_yield();

/* Terminate a thread */
void worker_exit(void* value_ptr);

/* Wait for thread termination */
int worker_join(worker_t thread, void** value_ptr);

/* Initialize the mutex lock */
int worker_mutex_init(worker_mutex_t* mutex, const pthread_mutexattr_t* mutexattr);

/* Acquire the mutex lock */
int worker_mutex_lock(worker_mutex_t* mutex);

/* Release the mutex lock */
int worker_mutex_unlock(worker_mutex_t* mutex);

/* Destroy the mutex */
int worker_mutex_destroy(worker_mutex_t* mutex);

/* Function to print global statistics. Do not modify this function. */
void print_app_stats(void);

/* Set thread priority (for MLFQ scheduling) */
#ifdef MLFQ
int worker_setschedprio(worker_t thread, int prio);
#endif

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#define pthread_setschedprio worker_setschedprio
#endif

/* Forward declarations */
static void threading_init();
static tcb* find_thread_by_id(worker_t thread_id);
static void schedule();
static void sched_psjf();
static void sched_mlfq();
static void timer_handler(int signum);
static void enqueue_thread(tcb* thread);
static tcb* dequeue_thread();
static void enqueue_thread_mlfq(tcb* thread);
static tcb* dequeue_thread_mlfq(int level);
static int all_queues_empty();
static void refresh_priorities();

#endif
