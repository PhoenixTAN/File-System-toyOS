#include "thread.h"
#include "multiboot.h"
#include "print.h"
#include "memos.h"
#include "heap.h"

#define N 3

void init(unsigned long addr)
{
    cls();
    println("File system DISCOS: Jiaqian Sun, Ziqi Tan");

    print_memory_map(addr);

    init_heap();
    init_pic();
    init_pit(); 
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

    __asm__ __volatile__("sti");
    yield();
    __asm__ __volatile__("cli");
    print("All threads finished.");
}


