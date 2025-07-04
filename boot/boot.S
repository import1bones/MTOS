#include "../include/mmu.h"
.set PROT_MODE_CSEG, 0x8
#kernel code segment selector
.set PROT_MODE_DSEG, 0x10
#kernel code segment selector
.set CR0_PE_ON, 0x1
#protected mode enable flag

.globl start
start:
	.code16
	cli
	cld

	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss

seta20.1:
	inb		$0x64, %al
	testb	$0x2, %al
	jnz		seta20.1

	movb	$0xd1, %al
	outb	%al, $0x64

seta20.2:
	inb		$0x64, %al
	testb	$0x2, %al
	jnz		seta20.2

	movb	$0xdf, %al
	outb	%al, $0x60
	
	lgdt	gdtdesc
	movl	%cr0, %eax
	orl		$CR0_PE_ON, %eax
	movl	%eax, %cr0

	ljmp	$PROT_MODE_CSEG, $protcseg

	.code32
protcseg:
	movw	$PROT_MODE_DSEG, $ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	movl	$start, %esp
	call	bootmain

	spin:
	jmp 	spin

.p2align 2
gdt:
	SEGMENT_NULL
	SEGMENT(SEGMENT_TYPE_X|SEGMENT_TYPE_R, 0x0, 0xffffffff)
	SEGMENT(SEGMENT_TYPE_W, 0x0, 0xffffffff)

gdtdesc:
	.word	0x17
	.long	gdt
