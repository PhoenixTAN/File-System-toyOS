#ifndef THREAD_H
#define THREAD_H

#include "queue.h"

typedef enum
{
    empty,
    new,
    ready,
    running
} State;

typedef struct
{
    unsigned TID;
    State state;
    unsigned *stack;
    unsigned *esp;
} TCB;

void init_thread(size_t size);
unsigned thread_create(void (*)());
unsigned get_tid(void);
void yield(void);

#endif
