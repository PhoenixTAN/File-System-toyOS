#include "multiboot.h"
#include "memos.h"
#include "print.h"

void print_memory_map(unsigned long addr) {
    multiboot_info_t *mbi;
    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;
    // init data
    multiboot_memory_map_t *mmap;
    multiboot_uint64_t memory_size = 0;
    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr; 
          (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length; 
            mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof(mmap->size))) {       
        // 64 bits base address, 64 bits length and 32 bits type.
        // add up free memory
        if (mmap->type == 1) {
            memory_size += mmap->len;   // the size of the memory region in bytes
        }
        print_memory_range(mmap->addr, mmap->len, mmap->type);
    }

    print_memory_size(memory_size);
}

void print_memory_size(multiboot_uint64_t memory_size) {
    // convert memory size in B to MB
    memory_size = memory_size >> 20;
    print("MemOS 2: Welcome *** System memory is (in MB): ");
    print_hex_string(memory_size);
    put_char('\n');
}

void print_memory_range(multiboot_uint64_t base_addr, multiboot_uint64_t len, multiboot_uint32_t type) {
    print("Address range: [");
    print_hex_string(base_addr);
    put_char('~');
    print_hex_string(base_addr + len - 1);
    print("] -> ");
    if( type == 1 ) {
        println("Free memory (1)");
    }
    else if( type == 2 ) {
        println("Reserved memory (2)");
    }
    else {
        println("Others");
    }
}
