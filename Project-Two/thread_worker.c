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
/* Runqueue definition */
queue_t* runqueue;

/* Function to add a thread to the runqueue */
void add_to_runqueue(tcb *new_tcb) 
{
    enqueue(runqueue, new_tcb);
}

/* Function to remove the head thread from the runqueue */
tcb* remove_from_runqueue() 
{
    return (tcb*)dequeue(runqueue);
}

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) 
{
    // Allocate memory for TCB
    tcb *new_tcb = (tcb *)malloc(sizeof(tcb));
    if (new_tcb == NULL) 
    {
        perror("Failed to allocate memory for TCB");
        return -1;
    }

    // Assign ID
    static worker_t next_id = 1;
    new_tcb->t_id = next_id++;
    *thread = new_tcb->t_id;

    new_tcb->t_status = THREAD_RUNNABLE;

    // Allocate stack for the thread
    new_tcb->t_stack = malloc(SIGSTKSZ);
    if (new_tcb->t_stack == NULL) 
    {
        perror("Failed to allocate stack for thread");
        free(new_tcb);
        return -1;
    }

    // Initialize context
    if (getcontext(&new_tcb->t_context) == -1) 
    {
        perror("Failed to get context");
        free(new_tcb->t_stack);
        free(new_tcb);
        return -1;
    }

    // Set up new context
    new_tcb->t_context.uc_stack.ss_sp = new_tcb->t_stack;
    new_tcb->t_context.uc_stack.ss_size = SIGSTKSZ;
    new_tcb->t_context.uc_link = NULL;

    // Make the context run and add the new thread to the runqueue
    makecontext(&new_tcb->t_context, (void (*)(void))function, 1, arg);
    add_to_runqueue(new_tcb);

    return 0;
}

/* yield the CPU to another thread */
int worker_yield() 
{
    tcb *current_tcb = remove_from_runqueue();
    if (current_tcb == NULL) return -1; 

    // Add the current thread back to the end of the runqueue
    add_to_runqueue(current_tcb);

    // Get the next thread from the runqueue
    tcb *next_tcb = (tcb *)top(runqueue);
    if (next_tcb == NULL) return -1; 

    // Set the current thread state to THREAD_RUNNABLE
    current_tcb->t_status = THREAD_RUNNABLE;

    // Set the next thread state to THREAD_RUNNABLE
    next_tcb->t_status = THREAD_RUNNABLE;

    // Swap context to the next thread
    if (swapcontext(&current_tcb->t_context, &next_tcb->t_context) == -1) 
    {
        perror("Failed to swap context");
        return -1;
    }

    return 0;
}

/* terminate a thread */
void worker_exit(void *value_ptr) 
{
    tcb *current_tcb = (tcb *)top(runqueue);
    if (current_tcb == NULL) return;

    // Set the thread state to THREAD_BLOCKED
    current_tcb->t_status = THREAD_BLOCKED;

    // Free the stack memory and TCB
    free(current_tcb->t_stack);
    free(current_tcb);

    // Remove from hash map
    deleteHash(current_tcb->t_id);

    // Get the next thread from the runqueue
    tcb *next_tcb = (tcb *)dequeue(runqueue);
    if (next_tcb != NULL) 
    {
        next_tcb->t_status = THREAD_RUNNABLE;
        setcontext(&next_tcb->t_context); // Switch to the next thread's context
    }

    exit(0);
}

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) 
{
    tcb *target_tcb = find(thread);
    if (target_tcb == NULL) return -1;

    // Wait until the target thread is terminated
    while (target_tcb->t_status != THREAD_BLOCKED) worker_yield();

    return 0;
}

#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {


   // Set the priority value to your thread's TCB
   // YOUR CODE HERE

   return 0;	

}
#endif

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

