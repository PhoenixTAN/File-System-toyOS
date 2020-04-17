#include "print.h"
#include "multiboot.h"

/* The number of columns. */
#define COLUMNS                 80
/* The number of lines. */
#define LINES                   24
/* The attribute of an character. */
#define ATTRIBUTE               7
/* The video memory address. */
#define VIDEO                   0xB8000

/* Variables. */
static int xpos;                        /* Save the X position (column). */
static int ypos;                        /* Save the Y position (row). */
static volatile unsigned char *video;   /* Point to the video memory. */

/* Clear the screen and initialize VIDEO, XPOS and YPOS. */
void cls (void) {
    int i;

    video = (unsigned char *) VIDEO;
    
    for (i = 0; i < COLUMNS * LINES * 2; i++) {
        *(video + i) = 0;
    }
    
    xpos = 0;
    ypos = 0;
}

void put_char(char ch) {
    if (ch == '\n' || ch == '\r') {
        newline();
        return;
    }

    *(video + (xpos + ypos * COLUMNS) * 2) = ch & 0xFF;       // ASCII
    *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;   // set up color

    xpos++;
    if (xpos >= COLUMNS) {
        newline();
    }       
}

void newline(void) {
    xpos = 0;
    ypos++;
    if (ypos >= LINES) {
        cls();
        ypos = 0;
    }
}

void print(char* str) {
    int i;
    for( i = 0; str[i] != '\0'; i++ ) {
        put_char(str[i]);
    }
}

void println(char* str) {
    int i;
    for( i = 0; str[i] != '\0'; i++ ) {
        put_char(str[i]);
    }
    newline();
}

void print_hex_string(multiboot_uint64_t data) {
    char hex_str[19];
    hex_str[0] = '0';
    hex_str[1] = 'x';
    hex_str[18] = '\0';
    multiboot_uint16_t index;
    multiboot_uint64_t temp;
    for( index = 17, temp = 0x0000000f; index > 1; index--, data = data >> 4 ) {
        multiboot_uint64_t hex_number = data & temp;
        if( hex_number > 9 ) {
            hex_number += 87;   // convert a~f to 'a'~'f'
        }
        else {
            hex_number += 48;
        }
        hex_str[index] = hex_number;
    }
    print(hex_str);
}
