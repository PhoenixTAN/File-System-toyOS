/**
 * Author: Ziqi Tan, Jiaqian Sun
 * 
 * Reference: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
*/

#include "multiboot.h"
#include "thread.h"
#include "print.h"
#include "interrupt.h"
#include "memos.h"
#include "queue.h"

/* Macros. */
/* The number of threads */
#define N                       3
#define stack_size              1024
#define ready_queue_size        256

/* Variables */
static TCB tcb[N];
static int uid = 0;     // counter
static TCB* lastThread = (void*)0;

Queue *ready_queue;

/* Private stack for threads */
multiboot_uint32_t stack1[stack_size];
multiboot_uint32_t stack2[stack_size];
multiboot_uint32_t stack3[stack_size];

/* Forward declarations. */
void init();
int create_thread(void* stack, void* run);
void thread1_run();
void thread2_run();
void thread3_run();
void delay();

/* scheduling */
void round_robin_scheduler();
void thread_finish();
void get_threads_ready();
TCB* get_next_thread();
void switch_thread();


/* entrance of the C code */
void init(unsigned long addr) {
    cls();  // clear the screen

    println("File system DISCOS: Jiaqian Sun, Ziqi Tan");

    print_memory_map(addr);
    
    init_heap();    // for malloc

    init_pic();
    init_pit();

    /* initialize ready queue */
    // init_ready_queue();
    ready_queue = init_queue(ready_queue_size);

    /* create threads */
    create_thread(&stack1[stack_size-1], thread1_run);
    create_thread(&stack2[stack_size-1], thread2_run);
    create_thread(&stack3[stack_size-1], thread3_run);

    __asm__ __volatile__("sti");
    // wait for interrupt
    delay();
}

/* Create thread */
int create_thread(void* stack, void* run) {
    // index out of bound exception
    if( uid > N ) {
        return -1;
    }
    tcb[uid].sp = (multiboot_uint32_t*)stack;
    tcb[uid].tid = uid + 1;
    tcb[uid].run = run;
    tcb[uid].status = NEW;
    tcb[uid].priority = 0;
    
    // initial context
    *((multiboot_uint32_t *)stack - 0) = (multiboot_uint32_t)run;   // EIP 
    // FLAG[1] 1，always 1 in EFLAGS
    // FLAG[9] 0，disable interrupt
    *((multiboot_uint32_t *)stack - 1) = 2;          // FLAG
    *((multiboot_uint32_t *)stack - 2) = 0;          // EAX   
    *((multiboot_uint32_t *)stack - 3) = 0;          // EBX 
    *((multiboot_uint32_t *)stack - 4) = 0;          // ECX 
    *((multiboot_uint32_t *)stack - 5) = 0;          // EDX
    *((multiboot_uint32_t *)stack - 6) = 0;          // ESP
    *((multiboot_uint32_t *)stack - 7) = 0;          // EBP 
    *((multiboot_uint32_t *)stack - 8) = 0;          // ESI 
    *((multiboot_uint32_t *)stack - 9) = 0;          // EDI
    *(((multiboot_uint16_t *)stack) - 19) = 0x10;    // SS
    *(((multiboot_uint16_t *)stack) - 20) = 0x10;    // DS
    *(((multiboot_uint16_t *)stack) - 21) = 0x10;    // ES
    *(((multiboot_uint16_t *)stack) - 22) = 0x10;    // FS
    *(((multiboot_uint16_t *)stack) - 23) = 0x10;    // GS
    // update the stack pointer at one time
    tcb[uid].sp = (multiboot_uint32_t)((multiboot_uint16_t*)stack - 23);
    uid++;

    return tcb[uid].tid;
}

/* do some cleaning jobs when a thread finish */
void thread_finish() {
    lastThread->status = TERMINATED;
    round_robin_scheduler(); 
}

/* simulate thread 1 */
void thread1_run() {
    int jobs = 2;
    while( jobs ) {
        print("Thread<0001> is running...  ");
        delay();
        jobs--;  
    }
    print("Thread<0001> finished.");
    thread_finish();
}

/* simulate thread 2 */
void thread2_run() {
    int jobs = 4;
    while( jobs ) {
        print("Thread<0002> is running...  ");
        delay();
        jobs--;       
    }
    print("Thread<0002> finished.");
    thread_finish();
}

/* simulate thread 3 */
void thread3_run() {
    int jobs = 7;
    while( jobs ) {
        print("Thread<0003> is running...  ");
        delay();
        jobs--;   
    }
    print("Thread<0003> finished.");
    thread_finish();
}

/* First come first serve scheduler */
void round_robin_scheduler() {
    
    // (1) add all NEW threads into ready queue.
    get_threads_ready();

    // (2) if the last thead has not terminated, add into ready queue.
    if ( lastThread != (void*)0 && lastThread->status != TERMINATED ) {
        ready_queue->add(ready_queue, lastThread);
        lastThread->status = READY;
    }
    
    // (3) ready queue is empty
    if ( ready_queue->isEmpty(ready_queue) ) {
        println("Ready queue is empty.");
        __asm__ volatile("jmp schedule_finish");
    }

    // (4) Find the next thread
    TCB* nextThread = get_next_thread();

    // (5) use asm to switch thread
    switch_thread(nextThread);

}

/* add new threads into ready queue */
void get_threads_ready() {

    int i;
    for ( i = 0; i < N; i++ ) {
        if( tcb[i].status == NEW /*|| tcb[i].status == READY*/ ) {
            ready_queue->add(ready_queue, &tcb[i]);
            /*print("add: ");
            put_char(tcb[i].tid + '0');*/
            tcb[i].status = READY;
        }
    }
}


/* get next thread from the ready queue */
TCB* get_next_thread() {
    // return de_queue();
    TCB* thread = (TCB*)ready_queue->poll(ready_queue);
    /*print("poll");
    put_char(thread->tid + '0');*/
    return thread;
}   


/* switch from last*/
void switch_thread(TCB* nextThread) {
    if( lastThread == (void*)0 || lastThread->status == TERMINATED ) {
        lastThread = nextThread;
        nextThread->status = RUNNING;
        __asm__ volatile("call context_retrieve"::"D"(nextThread));
    }
    else {
        TCB* temp = lastThread;     // record last thread
        lastThread = nextThread;    // update last thread
        
        nextThread->status = RUNNING;
        // print scheduling information
        /*
        print("from thread");
        put_char(temp->tid+'0');
        print(" to ");
        put_char(nextThread->tid + '0');
        println(""); 
        */    
        __asm__ volatile("call context_protection"::"S"(temp), "D"(nextThread));
        // "S": ESI, "D": EDI
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
