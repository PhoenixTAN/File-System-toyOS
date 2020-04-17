#ifndef PRINT_H
#define PRINT_H

#include "multiboot.h"

void cls (void);                    /* Clear the screen and initialize VIDEO, XPOS and YPOS. */
void put_char(char ch);             /* print a char */ 
void print(char* string);           /* print a string without switching to a new line*/
void println(char* string);         /* print a string and switch to a new line */
void newline(void);                 /* switch to a new line */
void print_hex_string(multiboot_uint64_t data);

#endif