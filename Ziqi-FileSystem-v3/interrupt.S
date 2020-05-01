    .globl init_pic
    .globl init_pit
    .globl enable_interrupt
    .globl disable_interrupt
    .globl default_handler
    .globl timer

#define ob(value, port) \
    movb value, %al; \
    outb %al, port

init_pic:
    # master
    ob($0x11, $0x20) # ICW1, double chips, with ICW4
    ob($0x20, $0x21) # ICW2, IRQ0=0x20
    ob($0x04, $0x21) # ICW3, accept slave at IRQ2
    ob($0x0d, $0x21) # ICW4, buffered, master, auto EOI, 8086/8088
    # slave
    ob($0x11, $0xa0)
    ob($0x28, $0xa1) # IRQ8=0x28
    ob($0x02, $0xa1)
    ob($0x09, $0xa1)
    ret

init_pit:
    ob($0x34, $0x43) # channel 0, low byte then high byte, mode 2 rate generator
    movw 4(%esp), %ax
    outb %al, $0x40 # low byte of interval clock cycles
    movb %ah, %al
    outb %al, $0x40 # high byte
    ret

enable_interrupt:
    sti
    ret

disable_interrupt:
    cli
    ret

default_handler:
    ob($0x20, $0x20) # EOI
    iret

timer:
    ob($0x20, $0x20)
    call stash
    iret
