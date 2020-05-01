#include "thread.h"
#include "interrupt.h"

#include "multiboot.h"
#include "print.h"
#include "memos.h"

#define N 3

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
    int jobs = 7;
    while( jobs ) {
        println("Thread<0003> is running...  ");
        delay();
        jobs--;   
    }
    println("Thread<0003> finished.");
}

void init(unsigned long addr)
{
    cls();
    println("File system DISCOS: Jiaqian Sun, Ziqi Tan");

    print_memory_map(addr);

    init_heap();
    init_pic();
    init_pit(11932); // 10ms interval
    init_thread(N*2);
    
    int tid = thread_create(thread1_run);
    if (tid) {
        print("Thread ");
        put_char(tid+'0');
    }
    else {
        print("Failed to create new thread.");
    }

    tid = thread_create(thread2_run);
    if (tid) {
        print("Thread ");
        put_char(tid+'0');
    }
    else {
        print("Failed to create new thread.");
    }

    tid = thread_create(thread3_run);
    if (tid) {
        print("Thread ");
        put_char(tid+'0');
    }
    else {
        print("Failed to create new thread.");
    }

    enable_interrupt();
    yield();
    disable_interrupt();
    print("All threads finished.");
}


