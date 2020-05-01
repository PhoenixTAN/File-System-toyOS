/*
 * boot.S for memos-2
 *
 */
	.globl stack
	.globl idt

	.bss
	.align 0x1000 # 4KB
	.comm stack, 0x1000 /* setup 4KB stack area in bss */
	.comm idt, 0x800 # 2KB for IDT

	.data
	.align 0x4

gdt:
	# null descriptor
	.long 0
	.long 0
	# code descriptor: base=0, limit=4GB
	.long 0x0000ffff
	.long 0x00cf9a00
	# data descriptor
	.long 0x0000ffff
	.long 0x00cf9200

idt_table:
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler

	.long timer
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler
	.long default_handler

gdt_ptr:
	.short 0x7ff # place for 256 descriptors
	.long gdt

idt_ptr:
	.short 0x7ff
	.long idt

	.text
	
	.globl _start

_start: 
	jmp real_start

	/* Multiboot header -- Safe to place this header in 1st page for GRUB */
	.align 4
	.long 0x1BADB002 /* Multiboot magic number */
	.long 0x00000003 /* Align modules to 4KB, req. mem size */
			 /* See 'info multiboot' for further info */
	.long 0xE4524FFB /* Checksum */

real_start:
	cli
	# switch to our own GDT
	lgdt gdt_ptr
	ljmp $0x08, $1f # code segment selector, index=1, TI=0 (GDT), RPL=0b00
1:
	movw $0x10, %ax # data segment selector, index=2
	movw %ax, %ss
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	
	/* set up stack */
	movl $stack+0x1000, %esp /* setup 4Kbyte stack */
	
	/* save multiboot parameter, for eventual call to C code */
	pushl %ebx

	movl $idt, %edi
	movl $0x30, %ecx
	movl $0x080000, %edx
	movl $idt_table, %esi
1:
	lodsl
	movw %ax, %dx
	movw $0xee00, %ax
	movl %edx, (%edi)
	addl $4, %edi
	stosl
	loop 1b

	lidt idt_ptr

	call init /* start of C code */

	/* In case we return from the call, we want to suspend the processor */
halt:
	cli
	hlt
