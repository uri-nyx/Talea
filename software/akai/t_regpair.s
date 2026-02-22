	.align 4
	.text
	.align 4
	.data
	.globl global_val
	.align	4
global_val:
	.word	0x11aabbcc
	.word	0xddeeffee
	.globl sink
	.align 4
	.text
	.align	4
sink:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sq x12,-12+16(x8) ; ASGNI8
L.1:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl main
	.align	4
main:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,global_val
	lq x12,0(x30) ; INDIR8
	li x14,170
	call sink
	mv x10,x0 ; LOADI4
L.2:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
