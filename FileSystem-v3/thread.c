#include "thread.h"

#define STACK_SIZE 4096

extern void stash(void); // save context
extern void trap(void);  // don't save context
extern void start(void);
extern void resume(void);

unsigned current_tid = 0;
unsigned **ptr_current_esp;
unsigned int pool_size;

TCB *thread_pool;
Queue *ready_queue;
unsigned **ptr_main_thread_esp;

void finish(void);

void init_thread(unsigned int size)
{
    pool_size = size;
    thread_pool = malloc(pool_size * sizeof(TCB));
    ptr_main_thread_esp = &thread_pool[0].esp;
    int i = 1;
    for (; i < pool_size; ++i)
    {
        thread_pool[i].TID = i;
        thread_pool[i].state = empty;
    }
    // ready_queue = new_FIFO_Queue(size);
    ready_queue = init_queue(size);
    current_tid = 0;
    ptr_current_esp = ptr_main_thread_esp = &thread_pool[0].esp;
}

unsigned thread_create(void (*f)(void))
{
    unsigned *stack;
    int i = 1;
    for (; i < pool_size; ++i)
    {
        if (thread_pool[i].state == empty)
        {
            break;
        }
    }
    if (i == pool_size || !(stack = (unsigned *)malloc(STACK_SIZE))) // 4KB for a thread
    {
        return 0;
    }
    thread_pool[i].state = new;
    thread_pool[i].stack = stack;
    stack = (unsigned *)((char *)stack + STACK_SIZE - 4); // must -4
    *stack-- = (unsigned)finish;                          // when thread finishes, call finish
    thread_pool[i].esp = stack;                           // set stack pointer
    *stack = (unsigned)f;                                 // for ret

    // ready_queue->push(ready_queue, thread_pool + i);
    ready_queue->add(ready_queue, thread_pool + i);
    return i;
}

unsigned get_tid()
{
    return current_tid;
}

void yield()
{
    stash();
}

void finish(void)
{
    thread_pool[current_tid].state = empty;
    free(thread_pool[current_tid].stack);
    current_tid = 0;
    trap();
}

void schedule()
{
    if (current_tid) // don't change state of main thread
    {
        thread_pool[current_tid].state = ready;
        // ready_queue->push(ready_queue, thread_pool + current_tid);
        ready_queue->add(ready_queue, thread_pool + current_tid);
    }
    // TCB *thread = (TCB *)ready_queue->pop(ready_queue);
    TCB *thread = (TCB *)ready_queue->poll(ready_queue);
    if (!thread) // no threads to run
    {
        current_tid = 0;
        ptr_current_esp = ptr_main_thread_esp;
        resume();
    }
    State state = thread->state;
    thread->state = running;
    current_tid = thread->TID;
    ptr_current_esp = &thread->esp;
    if (state == new)
    {
        start();
    }
    else
    {
        resume();
    }
}


/* simulate the running of the thread */
void delay() {
    int i, j;
    for( i = 0; i < 10000; i++ ) {
        for( j = 0; j < 6000; j++ ) {
        }
    }
}

/* simulate thread 1 */
void thread1_run() {
    int jobs = 2;
    while( jobs ) {
        println("Thread<0001> is running...  ");
        delay();
        jobs--;  
    }
    println("Thread<0001> finished.");
}

/* simulate thread 2 */
void thread2_run() {
    int jobs = 4;
    while( jobs ) {
        println("Thread<0002> is running...  ");
        delay();
        jobs--;       
    }
    println("Thread<0002> finished.");
}

/* simulate thread 3 */
void thread3_run() {
    int jobs = 6;
    while( jobs ) {
        println("Thread<0003> is running...  ");
        delay();
        jobs--;   
    }
    println("Thread<0003> finished.");
}

