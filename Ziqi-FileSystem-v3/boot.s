# -----------------------------------------------------------------------------------------------
# Author: 
#           Jiaqian Sun & Ziqi Tan
# Reference: 
#           memos-2 reference code
#           https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
#           http://quest.bu.edu/
# -----------------------------------------------------------------------------------------------

	.globl stack
	.globl idt

	.bss
	.align 0x1000 		# 4KB
	.comm stack, 0x1000 /* setup 4KB stack area in bss */
	.comm idt, 0x800 	# 2KB for IDT

	.data
	.align 0x4

gdt:
# (1) null descriptor -- 1st (Zeroth) entry not used
    .long 0
    .long 0

    # (2) Kernel CS
    # code descriptor
    .long 0x0000FFFF
    .long 0x00CF9A00
    # Base: 0x00000000; Limit: 0xFFFFF
    # Flags: 0xC(1100) -> 4KB blocks(page granularity) and 32 bit protected mode
    # Access Byte: 0x9A(10011010)
    #           Pr: 1; Privl: 00(highest); S(Descriptor type): 1; 
    #           Ex(Executable bit): 1 -> code selector

    # (3) Kernel DS
    # data descriptor
    .long 0x0000FFFF
    .long 0x00CF9200
    # Access Byte: 0x92(10010010)
    #           Ex(Executable bit): 0 -> data selector

    # (4) Task State Segment
    # (5) Other segments if needed: user-level, LDTs, TSS

idt_table:
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0

	.long timer
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0
	.long 0


gdt_ptr:
	.short 0x7FF # place for 256 descriptors
	.long gdt


idt_ptr:
	.short 0x7FF
	.long idt


.text
.globl _start

_start: 
	jmp real_start

	# Multiboot header -- Safe to place this header in 1st page for GRUB
	.align 4            # You are working on a 32-bit environment
	.long 0x1BADB002    # Multiboot magic number
	.long 0x00000003    # Align modules to 4KB, req. mem size
			            # See 'info multiboot' for further info
	.long 0xE4524FFB    # Checksum

real_start:
	cli
	# switch to our own GDT
	lgdt gdt_ptr
	ljmp $0x08, $1f # code segment selector, index=1, TI=0 (GDT), RPL=0b00
1:
    # set up kernel data segment for these registers
    movw $0x10, %AX
    movw %AX, %DS
    movw %AX, %SS
    movw %AX, %ES
    movw %AX, %FS
    movw %AX, %GS
	
	# set up stack
	movl $stack+0x1000, %ESP # setup 4Kbyte stack
	
	# save multiboot parameter, for eventual call to C code
	pushl %EBX

	movl $idt, %EDI
	movl $0x30, %ECX
	movl $0x080000, %EDX
	movl $idt_table, %ESI
1:
	lodsl
	movw %AX, %DX
	movw $0xee00, %AX
	movl %EDX, (%EDI)
	addl $4, %EDI
	stosl
	loop 1b

	lidt idt_ptr

	call init 		

schedule_finish:
	cli
	hlt		# halts CPU until the next external interrupt is fired

timer:
	movb    $0x20, %al
    outb    %al, $0x20
    call 	stash
    iret
