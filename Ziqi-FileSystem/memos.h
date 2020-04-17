#ifndef MEMOS_H
#define MEMOS_h

#include "multiboot.h"

void print_memory_map(unsigned long addr);
void print_memory_size(multiboot_uint64_t memory_size);
void print_memory_range(multiboot_uint64_t base_addr, multiboot_uint64_t len, multiboot_uint32_t type );

#endif