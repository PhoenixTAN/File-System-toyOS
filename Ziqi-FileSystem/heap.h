#ifndef HEAP_H
#define HEAP_H

#define NULL 0
typedef unsigned size_t;

void init_heap(void);
void *malloc(size_t);
void free(void *);

#endif
