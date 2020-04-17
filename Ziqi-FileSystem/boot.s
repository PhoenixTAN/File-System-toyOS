# -----------------------------------------------------------------------------------------------
# Author: 
#           Jiaqian Sun & Ziqi Tan
# Reference: 
#           memos-2 reference code
#           https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
#           http://quest.bu.edu/
# -----------------------------------------------------------------------------------------------

    .global stack

    .bss
    .align 0x1000
    .comm stack, 0x1000     # setup 4KB stack area in bss
    .comm idt, 0x800 /* interrupt descriptor table */

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

gdt_ptr:
    # Define the size of gdt
    .short 0x7FF    # length in bytes - 3 descriptors but space for 256 entries
    # Every entry is 8-byte-long. 256 entries are 0x800 bytes.

    # offset: 4 bytes
    .long gdt       # linear address of the table itself

idt_ptr:
	.short 0x7EE
	.long idt
 
	.text
	.global _start
	.global loop

.text
.global _start
.global schedule_finish

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
    lgdt gdt_ptr        # load gtd table address into GDTR register
    ljmp $0x08, $1f     # go to the kernel code segment

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
	# pushl %EBX

    pushl %ebx
	
	/* set up IDT by construction 64-bit interrupt desciptors 
	 * look at Ch.6 in the manual
	 */ 
	movl $idt, %edi
	movl $0x30, %ecx  /* set counter for 48 IDT entries used possibly 21 */
	movl $0x080000, %edx /* Used to index kernel CS */
	movl $int_table, %esi

    1:
	lodsl 			  /* value at address DS:ESI --> EAX */
	movw %ax, %dx     /* lower 16-bits  of int_table address (See: Fig 6.2) */
	movw $0x8E00, %ax /* DPL = (ring) 0 */
	movl %edx, (%edi) /* store bottom 32-bits of descriptor in EDX to IDT */
	addl $4, %edi
	stosl             /* store top 32-bits of descriptor held in EAX into IDT EAX --> ES:EDI */

	loop 1b

	lidt idt_ptr

    call init

schedule_finish:	
	cli
	hlt     # halts CPU until the next external interrupt is fired

int_table:
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
	
timer:
	call timer0_interrupt_handler
	iret

