// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "thread-worker.h"

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE

// Queue for the runqueue
queue_t *runqueue;


/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
    // Allocate memory for the TCB (Thread Control Block)
    tcb *new_thread = (tcb *)malloc(sizeof(tcb));
    if (new_thread == NULL) {
		printf("Error: Memory allocation failed for thread.\n");
        return -1;  
    }

    // Initialize the TCB
    new_thread->t_id = (worker_t)syscall(SYS_gettid);  // Assign a unique thread ID
    new_thread->t_status = THREAD_NEW;  // Mark thread as new
    new_thread->t_priority = DEFAULT_PRIO;  // Set default priority

    // Get the current context and prepare for the new thread
    if (getcontext(&(new_thread->t_context)) == -1) {
        free(new_thread);
		printf("Error: Could not get context.\n");
        return -1;  // Return error if context cannot be retrieved
    }

    // Allocate memory for the thread stack
    new_thread->t_stack = malloc(STACKSIZE);
    if (new_thread->t_stack == NULL) {
        free(new_thread);
		printf("Error: Memory allocation failed for thread stack.\n");
        return -1;  // Return error if stack allocation fails
    }

    // Set up the thread's context
    new_thread->t_context.uc_stack.ss_sp = new_thread->t_stack;
    new_thread->t_context.uc_stack.ss_size = STACKSIZE;
    new_thread->t_context.uc_stack.ss_flags = 0;
    new_thread->t_context.uc_link = NULL;  // Set to NULL to indicate no linked context

    // Make the context to point to the function to be executed by the thread
    makecontext(&(new_thread->t_context), (void (*)(void))function, 1, arg);

    // Add the new thread to the scheduler's runqueue
    enqueue(runqueue, new_thread);

    // Assign the thread ID to the provided thread pointer
    *thread = new_thread->t_id;

    // Set thread status to RUNNABLE
    new_thread->t_status = THREAD_RUNNABLE;

    return 0;  // Return success
}

#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {


   // Set the priority value to your thread's TCB
   // YOUR CODE HERE

   return 0;	

}
#endif

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

// YOUR CODE HERE

