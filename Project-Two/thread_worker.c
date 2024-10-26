#include "thread-worker.h"

/* Global variables for statistics */
long tot_cntx_switches = 0;
double avg_turn_time = 0;
double avg_resp_time = 0;

/* Initialize other global variables */
static tcb* current_thread = NULL;         // Currently running thread
static tcb* main_thread = NULL;            // Main thread TCB
static ucontext_t scheduler_context;       // Scheduler context
static runqueue_t runqueue = {NULL, NULL}; // Runqueue
static int thread_id_counter = 1;          // Thread ID counter
static int init = 0;                       // Initialization flag
static struct itimerval timer;             // Timer for scheduling
static struct sigaction sa;                // Signal handler
static tcb* all_threads = NULL;            // List of all threads
static int total_threads = 0;              // Total number of threads created

#define TIME_QUANTUM 10000                 // Time quantum in microseconds
#define REFRESH_TIME 100000                // Refresh time in microseconds
static mlfq_t mlfq;                        // Multi-level feedback queue
static int refresh_counter = 0;            // Counter for refresh

/* Initialize threading system */
static void threading_init() {
    if (init) return;
    init = 1;

    /* Save main context as main_thread */
    main_thread = (tcb*)malloc(sizeof(tcb));
    if (getcontext(&main_thread->context) == -1) {
        perror("getcontext");
        exit(1);
    }
    main_thread->thread_id = 0;
    main_thread->status = RUNNING;
    main_thread->priority = DEFAULT_PRIO;
    main_thread->first_scheduled = 1;
    main_thread->elapsed_time = 0;
    main_thread->retval = NULL;
    main_thread->next = NULL;
    main_thread->all_next = NULL;
    gettimeofday(&main_thread->creation_time, NULL);
    gettimeofday(&main_thread->first_scheduled_time, NULL);

    current_thread = main_thread;

    /* Add main_thread to all_threads list */
    all_threads = main_thread;
    total_threads = 1;

    /* Set up scheduler context */
    if (getcontext(&scheduler_context) == -1) {
        perror("getcontext");
        exit(1);
    }
    scheduler_context.uc_stack.ss_sp = malloc(SIGSTKSZ);
    scheduler_context.uc_stack.ss_size = SIGSTKSZ;
    scheduler_context.uc_stack.ss_flags = 0;
    scheduler_context.uc_link = &main_thread->context;
    makecontext(&scheduler_context, schedule, 0);

    /* Unblock SIGALRM */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    if (sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) {
        perror("Failed to unblock SIGALRM");
        exit(1);
    }

    /* Set up signal handler for timer */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_NODEFER | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Failed to set signal handler");
        exit(1);
    }

    /* Set up timer */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_QUANTUM;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_QUANTUM;
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Failed to set timer");
        exit(1);
    }
}

/* Create a new thread */
int worker_create(worker_t* thread, pthread_attr_t* attr, void* (*function)(void*), void* arg) {
    threading_init();

    /* Create Thread Control Block (TCB) */
    tcb* new_thread = (tcb*)malloc(sizeof(tcb));
    if (!new_thread) {
        perror("Failed to allocate TCB");
        return -1;
    }

    /* Initialize the context of this worker thread */
    if (getcontext(&new_thread->context) == -1) {
        perror("getcontext");
        free(new_thread);
        return -1;
    }
    new_thread->stack = malloc(SIGSTKSZ);
    if (!new_thread->stack) {
        perror("Failed to allocate stack");
        free(new_thread);
        return -1;
    }
    new_thread->context.uc_stack.ss_sp = new_thread->stack;
    new_thread->context.uc_stack.ss_size = SIGSTKSZ;
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_link = &scheduler_context; // Return to scheduler on exit

    /* Setup the context to execute the function */
    makecontext(&new_thread->context, (void (*)())function, 1, arg);

    /* Set thread attributes */
    new_thread->thread_id = thread_id_counter++;
    new_thread->status = READY;
    new_thread->priority = DEFAULT_PRIO;
    new_thread->first_scheduled = 0;
    new_thread->elapsed_time = 0;
    new_thread->retval = NULL;
    new_thread->next = NULL;
    new_thread->all_next = NULL;
    gettimeofday(&new_thread->creation_time, NULL);

    /* Assign thread ID to the output parameter */
    *thread = new_thread->thread_id;

    /* Add thread to the runqueue */
#ifdef MLFQ
    enqueue_thread_mlfq(new_thread);
#else
    enqueue_thread(new_thread);
#endif

    /* Add thread to all_threads list */
    new_thread->all_next = all_threads;
    all_threads = new_thread;
    total_threads++;

    return 0;
}

static int all_threads_exited() {
    tcb* thread = all_threads;
    while (thread) {
        if (thread->status != EXITED) {
            return 0; // Found a thread not yet EXITED
        }
        thread = thread->all_next;
    }
    return 1; // All threads have EXITED
}

/* Set the worker priority */
int worker_setschedprio(worker_t thread_id, int prio) {
    /* Set the priority value to your thread's TCB */
    tcb* thread = find_thread_by_id(thread_id);
    if (!thread) {
        return -1;
    }
    thread->priority = prio;
    return 0;
}

/* Yield CPU possession voluntarily */
int worker_yield() {
    /* Change thread status */
    current_thread->status = READY;

    /* Save context of this thread */
    if (getcontext(&current_thread->context) == -1) {
        perror("getcontext");
        exit(1);
    }

    /* Enqueue the current thread */
#ifdef MLFQ
    enqueue_thread_mlfq(current_thread);
#else
    enqueue_thread(current_thread);
#endif

    /* Switch to scheduler context */
    if (swapcontext(&current_thread->context, &scheduler_context) == -1) {
        perror("swapcontext");
        exit(1);
    }

    /* Execution resumes here when the thread is scheduled again */
    return 0;
}

/* Terminate a thread */
void worker_exit(void* value_ptr) {
    /* Set return value */
    current_thread->retval = value_ptr;
    current_thread->status = EXITED;

    /* Record end time */
    gettimeofday(&current_thread->end_time, NULL);

    /* Update statistics */
    double turnaround = (current_thread->end_time.tv_sec - current_thread->creation_time.tv_sec) * 1e6 +
                        (current_thread->end_time.tv_usec - current_thread->creation_time.tv_usec);
    avg_turn_time += turnaround;

    double response = (current_thread->first_scheduled_time.tv_sec - current_thread->creation_time.tv_sec) * 1e6 +
                      (current_thread->first_scheduled_time.tv_usec - current_thread->creation_time.tv_usec);
    avg_resp_time += response;

    /* Switch to scheduler context */
    if (swapcontext(&current_thread->context, &scheduler_context) == -1) {
        perror("swapcontext");
        exit(1);
    }
}

/* Wait for thread termination */
int worker_join(worker_t thread_id, void** value_ptr) {
    tcb* thread = find_thread_by_id(thread_id);
    if (!thread) {
        return -1;
    }

    /* Wait for the specified thread to terminate */
    while (thread->status != EXITED) {
        worker_yield();
    }

    if (value_ptr) {
        *value_ptr = thread->retval;
    }

    /* Clean up */
    free(thread->stack);
    // Optionally remove from all_threads list if you're no longer using it

    return 0;
}

/* Initialize the mutex lock */
int worker_mutex_init(worker_mutex_t* mutex, const pthread_mutexattr_t* mutexattr) {
    /* Initialize data structures for this mutex */
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
    return 0;
}

/* Acquire the mutex lock */
int worker_mutex_lock(worker_mutex_t* mutex) {
    /* Use atomic test-and-set to test the mutex */
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        /* Mutex is already locked, block the current thread */
        current_thread->status = BLOCKED;

        /* Add current thread to mutex wait queue */
        current_thread->next = mutex->wait_queue;
        mutex->wait_queue = current_thread;

        /* Switch to scheduler */
        if (swapcontext(&current_thread->context, &scheduler_context) == -1) {
            perror("swapcontext");
            exit(1);
        }
    }

    /* Mutex acquired successfully */
    mutex->owner = current_thread;
    return 0;
}

/* Release the mutex lock */
int worker_mutex_unlock(worker_mutex_t* mutex) {
    if (mutex->owner != current_thread) {
        /* Current thread does not own the mutex */
        return -1;
    }

    /* Release mutex */
    mutex->locked = 0;
    mutex->owner = NULL;

    /* Move threads in wait queue to runqueue */
    tcb* waiting_thread = mutex->wait_queue;
    while (waiting_thread) {
        tcb* next = waiting_thread->next;
        waiting_thread->status = READY;
#ifdef MLFQ
        enqueue_thread_mlfq(waiting_thread);
#else
        enqueue_thread(waiting_thread);
#endif
        waiting_thread = next;
    }
    mutex->wait_queue = NULL;

    return 0;
}

/* Destroy the mutex */
int worker_mutex_destroy(worker_mutex_t* mutex) {
    /* No dynamic memory to free in this implementation */
    return 0;
}

/* Scheduler */
static void schedule() {
    while (1) {
        tot_cntx_switches++;

        /* Check if all threads have exited */
        if (all_threads_exited()) {
            /* All threads have finished */
            avg_turn_time /= total_threads;
            avg_resp_time /= total_threads;
            print_app_stats();

            /* Switch back to main thread */
            if (setcontext(&main_thread->context) == -1) {
                perror("setcontext");
                exit(1);
            }

            /* Should not reach here */
            fprintf(stderr, "Error: setcontext to main_thread failed\n");
            exit(1);
        }

        /* Invoke scheduling algorithms according to the policy */
    #ifdef MLFQ
        sched_mlfq();
    #else
        sched_psjf();
    #endif
    }
}

/* Pre-emptive Shortest Job First scheduling algorithm */
static void sched_psjf() {
    /* Find the thread with the minimum elapsed_time */
    tcb* min_thread = NULL;
    tcb* prev = NULL;
    tcb* curr = runqueue.head;
    tcb* min_prev = NULL;

    while (curr) {
        if (curr->status == READY) {
            if (!min_thread || curr->elapsed_time < min_thread->elapsed_time) {
                min_thread = curr;
                min_prev = prev;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    if (min_thread) {
        /* Remove min_thread from runqueue */
        if (min_prev) {
            min_prev->next = min_thread->next;
        } else {
            runqueue.head = min_thread->next;
        }
        if (min_thread == runqueue.tail) {
            runqueue.tail = min_prev;
        }

        /* Update thread status */
        min_thread->status = RUNNING;

        /* Update response time if first scheduled */
        if (!min_thread->first_scheduled) {
            gettimeofday(&min_thread->first_scheduled_time, NULL);
            min_thread->first_scheduled = 1;
        }

        /* Switch to the thread */
        tcb* prev_thread = current_thread;
        current_thread = min_thread;
        if (swapcontext(&scheduler_context, &min_thread->context) == -1) {
            perror("swapcontext");
            exit(1);
        }
    } else {
        /* No threads ready, check if all threads have finished */
        if (current_thread->status == EXITED && runqueue.head == NULL) {
            /* All threads have finished */
            avg_turn_time /= total_threads;
            avg_resp_time /= total_threads;
            print_app_stats();
            exit(0);
        }

        /* Resume current thread if it's ready */
        if (current_thread->status == READY || current_thread->status == RUNNING) {
            if (swapcontext(&scheduler_context, &current_thread->context) == -1) {
                perror("swapcontext");
                exit(1);
            }
        }
    }
}

#ifdef MLFQ
/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
    /* Implement MLFQ scheduling */
    tcb* next_thread = NULL;
    for (int level = MAX_LEVELS - 1; level >= 0; level--) {
        if (mlfq.queues[level].head != NULL) {
            next_thread = dequeue_thread_mlfq(level);
            break;
        }
    }

    if (next_thread) {
        /* Update thread status */
        next_thread->status = RUNNING;

        /* Update response time if first scheduled */
        if (!next_thread->first_scheduled) {
            gettimeofday(&next_thread->first_scheduled_time, NULL);
            next_thread->first_scheduled = 1;
        }

        /* Switch to the thread */
        tcb* prev_thread = current_thread;
        current_thread = next_thread;
        if (swapcontext(&scheduler_context, &next_thread->context) == -1) {
            perror("swapcontext");
            exit(1);
        }
    } else {
        /* No threads ready, check if all threads have finished */
        if (current_thread->status == EXITED && all_queues_empty()) {
            /* All threads have finished */
            avg_turn_time /= total_threads;
            avg_resp_time /= total_threads;
            print_app_stats();
            exit(0);
        }

        /* Resume current thread if it's ready */
        if (current_thread->status == READY || current_thread->status == RUNNING) {
            if (swapcontext(&scheduler_context, &current_thread->context) == -1) {
                perror("swapcontext");
                exit(1);
            }
        }
    }

    /* Refresh priorities periodically */
    refresh_counter += TIME_QUANTUM;
    if (refresh_counter >= REFRESH_TIME) {
        refresh_priorities();
        refresh_counter = 0;
    }
}
#endif

/* Timer signal handler */
static void timer_handler(int signum) {
    /* Do not use printf inside signal handler */
    // const char msg[] = "Timer handler invoked\n";
    // write(STDERR_FILENO, msg, sizeof(msg) - 1);

    if (current_thread != main_thread && current_thread->status == RUNNING) {
        /* Preempt the current thread */
        current_thread->status = READY;
#ifdef MLFQ
        int level = current_thread->priority;
        if (level > 0) {
            current_thread->priority--;
        }
        enqueue_thread_mlfq(current_thread);
#else
        enqueue_thread(current_thread);
#endif
        if (swapcontext(&current_thread->context, &scheduler_context) == -1) {
            perror("swapcontext");
            exit(1);
        }
    }
}

/* Enqueue thread to runqueue */
static void enqueue_thread(tcb* thread) {
    if (!runqueue.head) {
        runqueue.head = runqueue.tail = thread;
    } else {
        runqueue.tail->next = thread;
        runqueue.tail = thread;
    }
    thread->next = NULL;
}

/* Dequeue thread from runqueue */
static tcb* dequeue_thread() {
    tcb* thread = runqueue.head;
    if (thread) {
        runqueue.head = thread->next;
        if (!runqueue.head) {
            runqueue.tail = NULL;
        }
        thread->next = NULL;
    }
    return thread;
}

#ifdef MLFQ
/* Enqueue thread to MLFQ */
static void enqueue_thread_mlfq(tcb* thread) {
    int level = thread->priority;
    runqueue_t* queue = &mlfq.queues[level];
    if (!queue->head) {
        queue->head = queue->tail = thread;
    } else {
        queue->tail->next = thread;
        queue->tail = thread;
    }
    thread->next = NULL;
}

/* Dequeue thread from MLFQ */
static tcb* dequeue_thread_mlfq(int level) {
    runqueue_t* queue = &mlfq.queues[level];
    tcb* thread = queue->head;
    if (thread) {
        queue->head = thread->next;
        if (!queue->head) {
            queue->tail = NULL;
        }
        thread->next = NULL;
    }
    return thread;
}

/* Check if all queues are empty */
static int all_queues_empty() {
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (mlfq.queues[i].head != NULL) {
            return 0;
        }
    }
    return 1;
}

/* Refresh priorities in MLFQ */
static void refresh_priorities() {
    /* Move all threads to the highest priority queue */
    for (int level = 1; level < MAX_LEVELS; level++) {
        tcb* thread = mlfq.queues[level].head;
        while (thread) {
            tcb* next = thread->next;
            thread->priority = MAX_LEVELS - 1;
            enqueue_thread_mlfq(thread);
            thread = next;
        }
        mlfq.queues[level].head = mlfq.queues[level].tail = NULL;
    }
}
#endif

/* Find thread by ID */
static tcb* find_thread_by_id(worker_t thread_id) {
    /* Search in all_threads list */
    tcb* thread = all_threads;
    while (thread) {
        if (thread->thread_id == thread_id) {
            return thread;
        }
        thread = thread->all_next;
    }
    return NULL;
}

/* Function to print global statistics. Do not modify this function. */
void print_app_stats(void) {
    fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
    fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
    fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}

