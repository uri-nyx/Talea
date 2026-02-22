	.align 4
	.text
# Compiled with lcc-sirius 4.2
# assemble with supplied as assembler
# link with supplied ld linker
	.globl struct_val
	.align	4
struct_val:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	sw x14,40(x8)
	li x30,512
	sw x30,0(x12)
	li x30,4
	add x30,x12,x30
	li x29,1024
	sw x29,0(x30)
	li x30,4
	add x30,x12,x30
	lw x30,0(x30)
	sw x30,-24+32(x8)
	sw x13,-28+32(x8)
	lw x30,8+32(x8)
	sw x30,-16+32(x8)
	lw x30,-8+32(x8)
	addi x5,x8,-28+32
	li x28,20
	copy x5,x30,x28
L.1:
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.6:
	.word	0x0
	.word	0x1
	.word	0x2
	.word	0x3f800000
	.word	0xbf800000
	.globl main
	.align 4
	.text
	.align	4
main:
	addi x2,x2,-96
	sw  x8,92(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	addi x30,x8,-44+64
	la x5,L.6
	li x28,20
	copy x5,x30,x28
	addi x30,x8,-64+64
	addi x5,x8,-44+64
	li x28,20
	copy x5,x30,x28
	addi x12,x8,-64+64
	li x13,18
	la x30,L.7
	lw x14,0(x30)
	call struct_val
	li x30,100
	sw x30,-24+64(x8)
	mv x10,x0 ; LOADI4
L.5:
	lw x1,24(x2)
	lw  x8,92(x2)
	addi  x2,x2,96
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.7:
	.word	0x422147ae
	.align 4
