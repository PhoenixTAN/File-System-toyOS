#ifndef QUEUE_H
#define QUEUE_H

#include "heap.h"

typedef struct Q Queue;
struct Q
{
    int (*push)(Queue *, void *);
    void *(*pop)(Queue *);
};

Queue* new_FIFO_Queue(size_t);

#endif
