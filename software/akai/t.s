	.align 4
	.text
	.globl main
	.align	4
main:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	mv x0,x0
	mv x10,x0 ; LOADI4
L.1:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align 4
