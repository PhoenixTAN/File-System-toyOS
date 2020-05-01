#include "queue.h"
#include "heap.h"

#define NEXT(i, size) (((i) + 1) % (size))

typedef struct
{
    Queue queue;
    void **list;
    size_t size;
    size_t start;
    size_t end;
} FIFO_Queue;

int FIFO_Queue_push(Queue *queue, void *element)
{
    FIFO_Queue *this = (FIFO_Queue *)queue;
    if (NEXT(this->end, this->size) == this->start)
    {
        return 0;
    }
    this->list[this->end] = element;
    this->end = NEXT(this->end, this->size);
    return 1;
}

void *FIFO_Queue_pop(Queue *queue)
{
    FIFO_Queue *this = (FIFO_Queue *)queue;
    if (this->end == this->start)
    {
        return NULL;
    }
    void *element = this->list[this->start];
    this->start = NEXT(this->start, this->size);
    return element;
}

Queue *new_FIFO_Queue(size_t size)
{
    ++size;
    FIFO_Queue *fifo_queue = malloc(sizeof(FIFO_Queue));
    if (!fifo_queue || !(fifo_queue->list = malloc(size * sizeof(void *))))
    {
        return NULL;
    }
    fifo_queue->queue.push = FIFO_Queue_push;
    fifo_queue->queue.pop = FIFO_Queue_pop;
    fifo_queue->size = size;
    fifo_queue->start = 0;
    fifo_queue->end = 0;
    return (Queue *)fifo_queue;
}
