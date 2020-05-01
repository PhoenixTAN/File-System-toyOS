#include "queue.h"
#include "heap.h"


int queue_isFull(Queue* queue) {
    FIFO_Queue *this = (FIFO_Queue *)queue;
    // the queue is full
    if( ((this->tail + 1)%this->size) == this->head  ) {
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


int queue_add(Queue *queue, void *element)
{
    if ( queue_isFull(queue) ) {
        return 0;
    }
    FIFO_Queue *this = (FIFO_Queue *)queue;

    
    this->elements[this->tail] = element;
    // this->tail = NEXT(this->tail, this->size);
    this->tail = (this->tail + 1) % this->size;
    return 1;
}


void *queue_poll(Queue *queue)
{   
    if ( queue_isEmpty(queue) ) {
        return NULL;
    }
    FIFO_Queue *this = (FIFO_Queue *)queue;

    void *element = this->elements[this->head];
    // this->head = NEXT(this->head, this->size);
    this->head = (this->head + 1) % this->size;
    return element;
}


Queue *init_queue(unsigned int size)
{
    ++size;
    FIFO_Queue *fifo_queue = malloc(sizeof(FIFO_Queue));
    if (!fifo_queue || !(fifo_queue->elements = malloc(size * sizeof(void *))))
    {
        return NULL;
    }
    fifo_queue->queue.add = queue_add;
    fifo_queue->queue.poll = queue_poll;

    fifo_queue->queue.isFull = queue_isFull;
    fifo_queue->queue.isEmpyt = queue_isEmpty;

    fifo_queue->size = size;
    fifo_queue->head = 0;
    fifo_queue->tail = 0;

    return (Queue *)fifo_queue;
}
