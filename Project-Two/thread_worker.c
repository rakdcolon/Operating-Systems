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

runqueue_t runqueue = {NULL, NULL, 0};

/* Function to add a thread to the runqueue */
void enqueue(tcb *new_tcb) 
{
    new_tcb->next = NULL;

    if (runqueue.tail == NULL) 
    {
        // Runqueue is empty
        runqueue.head = new_tcb;
        runqueue.tail = new_tcb;
    } 
    else 
    {
        // Add to the end of the runqueue
        runqueue.tail->next = new_tcb;
        runqueue.tail = new_tcb;
    }
    
    runqueue.size++;
}

/* Function to remove the head thread from the runqueue */
tcb* dequeue()
{
    if (runqueue.head == NULL) 
    {
        return NULL;
    }

    tcb *removed_tcb = runqueue.head;
    runqueue.head = runqueue.head->next;

    if (runqueue.head == NULL) 
    {
        runqueue.tail = NULL;
    }

    runqueue.size--;
    return removed_tcb;
}

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) 
{
    // Allocate memory for the new TCB
    tcb *new_tcb = (tcb *)malloc(sizeof(tcb));
    if (new_tcb == NULL) 
    {
        perror("Failed to allocate memory for TCB");
        return -1;
    }

    // Assign a unique thread ID
    static worker_t next_id = 1;
    new_tcb->id = next_id++;
    *thread = new_tcb->id;

    // Set thread state to READY
    new_tcb->state = READY;

    // Allocate stack for the thread
    new_tcb->stack = malloc(STACKSIZE);
    if (new_tcb->stack == NULL) 
    {
        perror("Failed to allocate stack for thread");
        free(new_tcb);
        return -1;
    }

    // Initialize the thread context
    if (getcontext(&new_tcb->context) == -1) 
    {
        perror("Failed to get context");
        free(new_tcb->stack);
        free(new_tcb);
        return -1;
    }

    // Set up the new context
    new_tcb->context.uc_stack.ss_sp = new_tcb->stack;
    new_tcb->context.uc_stack.ss_size = STACKSIZE;
    new_tcb->context.uc_link = NULL; // When the thread finishes, there's no continuation context

    // Make the context run the given function
    makecontext(&new_tcb->context, (void (*)(void))function, 1, arg);

    // Add the new thread to the runqueue
    enqueue(new_tcb);

    return 0;
}

/* yield the CPU to another thread */
int worker_yield() 
{
    tcb *current_tcb = dequeue();
    if (current_tcb == NULL) return -1; // No threads available to yield to

    // Add the current thread back to the end of the runqueue
    enqueue(current_tcb);

    // Get the next thread from the runqueue
    tcb *next_tcb = runqueue.head;
    if (next_tcb == NULL) return -1; // No threads to schedule

    // Set the current thread state to READY and the next thread state to RUNNING
    current_tcb->state = READY;
    next_tcb->state = RUNNING;

    // Swap context to the next thread
    if (swapcontext(&current_tcb->context, &next_tcb->context) == -1) 
    {
        perror("Failed to swap context");
        return -1;
    }

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

