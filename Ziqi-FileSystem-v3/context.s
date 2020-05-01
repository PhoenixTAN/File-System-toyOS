    .globl stash
    .globl trap
    .globl resume
    .globl start

stash:
    cli
    pushal
    pushfl
    pushw %ss
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs
    movl ptr_current_esp, %eax # now eax is &thread_pool[current_tid].esp
    movl %esp, (%eax) # store esp
trap:
    movl ptr_main_thread_esp, %eax
    movl (%eax), %esp # go back to thread 0
    call schedule

resume:
    movl ptr_current_esp, %eax
    movl (%eax), %esp
    popw %gs
    popw %fs
    popw %es
    popw %ds
    popw %ss
    popfl
    popal
    sti
    ret # from perspective of thread, the call to stash (thus yield) returns

start:
    movl ptr_current_esp, %eax
    movl (%eax), %esp # now esp points to address of the function
    sti
    ret
