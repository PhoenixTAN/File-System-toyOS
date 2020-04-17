#ifndef QUEUE_H
#define QUEUE_H

typedef struct {

    void** elements;

    unsigned int max_size;
    unsigned int head;
    unsigned int tail;

    int (*add)(Queue* queue, void *object);
    void *(*poll)(Queue* queue);
    
} Queue;

Queue* init_queue(unsigned int size);
int queue_add(Queue* queue, void* object);
void* queue_poll(Queue* queue);

#endif
