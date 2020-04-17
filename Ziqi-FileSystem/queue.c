#include "heap.h"
#include "queue.h"

int queue_add(Queue* queue, void* object) {

    // the queue is full
    if( ((queue->tail + 1)%queue->max_size) == queue->head  ) {
        return -1;
    }

    queue->elements[queue->tail] = object;
    queue->tail = (queue->tail + 1) % queue->max_size;

    return 0;
}

void* queue_poll(Queue* queue) {

    // the queue is empty
    if( queue->head == queue->tail ) {
        return NULL;
    }

    void* object = queue->elements[queue->head];
    queue->head = (queue->head + 1) % queue->max_size;
    return object;
}

Queue* init_queue(unsigned int size) {
    
    Queue* queue = malloc(sizeof(Queue));
    // malloc failed
    if( !queue ) {
        return NULL;
    }

    queue->elements = malloc(size * sizeof(void *));
    // malloc failed
    if( !queue->elements ) {
        return NULL;
    }

    queue->max_size = size;
    queue->head = 0;
    queue->tail = 0;

    queue->add = queue_add;
    queue->poll = queue_poll;

    return queue;
}
