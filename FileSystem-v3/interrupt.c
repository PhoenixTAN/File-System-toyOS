#include "interrupt.h"
#include "multiboot.h"

/* Macros */
#define PIC1		        0x20		/* IO base address for master PIC */
#define PIC2		        0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	    PIC1
#define PIC1_DATA	        (PIC1+1)
#define PIC2_COMMAND	    PIC2
#define PIC2_DATA	        (PIC2+1)

#define PIC_EOI		        0x20		/* End-of-interrupt command code */
#define PIC1_BASE_IRQ       0x20


/* PIT Macros */
#define FREQUENCY           20          /* in Hz, 54.9254 milisecond is the maximum int frequency */
#define PIT_FREQ            1193181     /* in Hz */
#define PIT_CONTROL_WORD    0x43
#define PIT_CHANNEL0        0x40

/* write data to port usPort */
static void outb (multiboot_uint8_t uch, multiboot_uint16_t usPort) {
    __asm__ __volatile__("outb %0, %1"::"a"(uch), "Nd"(usPort));
}

/* 
    8253/8254 chip Programmable Interval Timer 
    Oscillator: 1.193182 MHz
    Frequency divider: 16 bits
*/
void init_pit() {
    /*
        I/O port     Usage
        0x40         Channel 0 data port (read/write)
        0x41         Channel 1 data port (read/write)
        0x42         Channel 2 data port (read/write)
        0x43         Mode/Command register (write only, a read is ignored)
    */

    /* set mode reigister 0x43 */
    outb(0x34, PIT_CONTROL_WORD);   
    /*
        0x34 0011 0100
        00  ->  channel 0
        11  ->  Access mode: lobyte/hibyte
        010 ->  Opearting mode: rate generator. 
                This mode operates as a frequency divider.
        0   ->  16 bits binary mode
    */

    outb((multiboot_uint8_t)(PIT_FREQ / FREQUENCY & 0x00FF), PIT_CHANNEL0);    // lobyte
    outb((multiboot_uint8_t)(PIT_FREQ / FREQUENCY >> 8), PIT_CHANNEL0);    // hibyte
}

/* 8259 Programmable Interrupt Controller */
void init_pic() {
    /* set up master chip */
    outb(0x11, 0x20);           /* 8259 (ICW1) - xxx10x01 */
    outb(0x20, 0x21);     /* 8259 (ICW2) - set IRQ0... to int 0x20... */
    outb(0x0D, 0x21);              /* 8259 (ICW4) - Buffered master, normal EOI, 8086 mode */
}


