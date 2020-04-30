#include "heap.h"
#include "queue.h"
#include "print.h"


typedef struct {

    Queue queue;
    void **elements;
    unsigned int max_size;
    unsigned int head;
    unsigned int tail;

} FIFO_Queue;


/* functions */
Queue* init_queue(unsigned int size);
int queue_isFull(Queue* queue);
int queue_isEmpty(Queue* queue);
int queue_add(Queue* queue, void* object);
void* queue_poll(Queue* queue);


int queue_isFull(Queue* queue) {
    FIFO_Queue *this = (FIFO_Queue *)queue;
    // the queue is full
    if( ((this->tail + 1)%this->max_size) == this->head  ) {
        println("Queue is full.");
        return 1;
    }
    return 0;
}


int queue_isEmpty(Queue* queue) {
    FIFO_Queue *this = (FIFO_Queue *)queue;

    // the queue is empty
    if( this->head == this->tail ) {
        println("Queue is empty.");
        return 1;
    }
    return 0;
}


int queue_add(Queue* queue, void* object) {
    
    if ( queue_isFull(queue) ) {
        return -1;
    }

    FIFO_Queue *this = (FIFO_Queue *)queue;

    this->elements[this->tail] = object;
    this->tail = (this->tail + 1) % this->max_size;
    
    return 0;
}



void* queue_poll(Queue* queue) {

    if ( queue_isEmpty(queue) ) {
        return -1;
    }

    FIFO_Queue *this = (FIFO_Queue *)queue;
 
    void* object = this->elements[this->head];
    this->head = (this->head + 1) % this->max_size;
    return object;
}


Queue* init_queue(unsigned int size) {
    
    FIFO_Queue *fifo_queue = malloc(sizeof(FIFO_Queue));
    
    // malloc failed
    if ( !fifo_queue ) {
        println("Malloc failed: fifo_queue.");
        return NULL;
    }

    fifo_queue->elements = malloc(size*sizeof(void *));
    // malloc failed
    if ( !fifo_queue->elements ) {
        println("Malloc failed: queue.");
        return NULL;
    }

    fifo_queue->queue.add = queue_add;
    fifo_queue->queue.poll = queue_poll;
    fifo_queue->queue.isFull = queue_isFull;
    fifo_queue->queue.isEmpty= queue_isEmpty;

    fifo_queue->max_size = size;

    fifo_queue->head = 0;
    fifo_queue->tail = 0;

    return (Queue *)fifo_queue;
}


// queue test
// put this code in a driver function
/*
    // queue test
    TCB *thread;
    thread = (TCB *)ready_queue->poll(ready_queue); // null

    if ( ready_queue->isFull(ready_queue) ) {
        println("ready queue is full.");
    }

    if ( ready_queue->isEmpty(ready_queue) ) {
        println("ready queue is Empty.");
    }

    ready_queue->add(ready_queue, &tcb[0]);
    ready_queue->add(ready_queue, &tcb[1]);
    thread = (TCB *)ready_queue->poll(ready_queue);
    put_char(thread->tid + '0');    // 1
    
    ready_queue->add(ready_queue, &tcb[2]);

    thread = (TCB *)ready_queue->poll(ready_queue);
    put_char(thread->tid + '0');    // 2
    
    thread = (TCB *)ready_queue->poll(ready_queue);
    put_char(thread->tid + '0');    // 3

    thread = (TCB *)ready_queue->poll(ready_queue); // null

    // full
    ready_queue->add(ready_queue, &tcb[0]);
    ready_queue->add(ready_queue, &tcb[1]);
    ready_queue->add(ready_queue, &tcb[1]);
    ready_queue->add(ready_queue, &tcb[1]); // full

    if ( ready_queue->isFull(ready_queue) ) {
        println("ready queue is full.");
    }

    if ( ready_queue->isEmpty(ready_queue) ) {
        println("ready queue is Empty.");
    }
*/
