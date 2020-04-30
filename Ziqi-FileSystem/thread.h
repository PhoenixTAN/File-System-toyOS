#ifndef THREAD_H
#define THREAD

/* Thead status */
typedef enum  {
    NEW, READY, RUNNING, BLOCK, TERMINATED
} THREAD_STATUS;

/* TCB(Thread Control Block) */
typedef struct {
    unsigned int *sp;           // stack pointer
    int tid;                    // a thread uid
    unsigned int (*run)();
    THREAD_STATUS status;
    int priority;

} TCB;


#endif
