/* Thead status */
enum THREAD_STATUS {
    NEW, READY, RUNNING, BLOCK, TERMINATED
};

/* TCB(Thread Control Block) */
typedef struct {
    unsigned int *sp;           // stack pointer
    int tid;                    // a thread uid
    unsigned int (*run)();
    enum THREAD_STATUS status;
    int priority;

} TCB;


