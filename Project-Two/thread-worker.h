// File:	worker_t.h

// List all group member's name:
// username of iLab:
// iLab Server:

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

typedef uint worker_t;

typedef enum {
	THREAD_NEW,
	THREAD_RUNNABLE,
	THREAD_BLOCKED,
	THREAD_WAITING,
} thread_status_t;

typedef struct TCB {
	/* add important states in a thread control block */
	worker_t t_id;
	// status can be any one of the enumerated values above
	thread_status_t t_status;
	ucontext_t t_context;
	void* t_stack;
	int t_priority;
	// And more ...

	// YOUR CODE HERE
	
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */
	
	// YOUR CODE HERE
} worker_mutex_t;

/* Priority definitions */
#define NUMPRIO 4

#define HIGH_PRIO 3
#define MEDIUM_PRIO 2
#define DEFAULT_PRIO 1
#define LOW_PRIO 0

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE
typedef struct node_t {
	void* data;
	struct node_t* next;
} node_t;

typedef struct queue_t {
	struct node_t* head;
	struct node_t* end;
	int size;
	size_t dataSize;
} queue_t;

/* Function Declarations: */
node_t* makeNode(void* data, size_t dataSize){
	node_t* newNode = (node_t*)malloc(sizeof(node_t));
	newNode->data = malloc(dataSize); // allocate memory for data
	memcpy(newNode->data, data, dataSize); // copy data into node
	newNode->next = NULL;
	return newNode;
}

queue_t* makeQueue(size_t dS){
	queue_t* q = (queue_t*)malloc(sizeof(queue_t));
	q->head = NULL;
	q->end = NULL;
	q->size = 0;
	q->dataSize = dS;
	return q;
}

void enqueue(queue_t* q,void* data){
	node_t* newNode = makeNode(data, q->dataSize);
	if (q->size == 0){
		q->head = newNode;
		q->end = newNode;
	} else{
		q->end->next = newNode;
		q->end = newNode;
	}
	q->size += 1;
}

void* dequeue(queue_t* q){
	if (q->size == 0){
		printf("Queue is empty!\n");
		return NULL;
	}
	void* data = q->head->data;
	q->head = q->head->next;
	if(q->head == NULL){
		q->end = NULL;
	}
	q->size -= 1;
	return data;
}

void* top(queue_t* q){
	return q->head;
}

void freeQueue(queue_t* q){
	node_t* tempNode;
	while(top(q) != NULL){
		tempNode = dequeue(q);
		free(tempNode->data);
		free(tempNode);
	}
	free(q);
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

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

#endif
