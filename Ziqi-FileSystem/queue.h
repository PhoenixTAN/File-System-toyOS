#ifndef QUEUE_H
#define QUEUE_H

#include "heap.h"

typedef struct Q Queue;
struct Q
{
    int (*add)(Queue *, void *);
    void *(*poll)(Queue *);
    int (*isFull)(Queue *);
    int (*isEmpty)(Queue *);
};

Queue* init_queue(unsigned int size);

#endif
