#ifndef QUEUE_H
#define QUEUE_H


typedef struct Q Queue;
struct Q
{
    int (*add)(Queue *, void *);
    void *(*poll)(Queue *);
    int (*isFull)(Queue *);
    int (*isEmpyt)(Queue *);
};


typedef struct
{
    Queue queue;
    void **elements;
    unsigned int size;
    unsigned int head;
    unsigned int tail;

} FIFO_Queue;


Queue* init_queue(unsigned int);


#endif

