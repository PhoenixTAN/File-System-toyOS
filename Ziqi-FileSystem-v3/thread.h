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


void init_thread(unsigned int size);
unsigned thread_create(void (*)());
unsigned get_tid(void);
void yield(void);


void thread1_run();
void thread2_run();
void thread3_run();
void delay();

#endif
