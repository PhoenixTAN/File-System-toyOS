#ifndef INTERRUPT_H
#define INTERRUPT_H

void init_pic(void);
void init_pit(unsigned short counter);
void enable_interrupt(void);
void disable_interrupt(void);

#endif
