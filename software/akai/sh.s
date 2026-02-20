	.align 4
	.text
	.globl tokenize
	.align	4
tokenize:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	mv x10,x0 ; LOADI4
L.8:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.10:
	.byte	0x7
	.align	1
L.11:
	.byte	0x1
	.align	1
L.12:
	.byte	0x1b
	.byte	0x5b
	.byte	0x32
	.byte	0x4a
	.byte	0x0
	.globl main
	.align 4
	.text
	.align	4
main:
	addi x2,x2,-336
	sw  x8,332(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	addi x30,x8,-261+288
	la x5,L.10
	lb x28,0(x5)
	sb	x28, 0(x30)
	addi x30,x8,-262+288
	la x5,L.11
	lb x28,0(x5)
	sb	x28, 0(x30)
	addi x30,x8,-267+288
	la x5,L.12
	lb x28,0(x5)
	lb x6,1(x5)
	sb	x28, 0(x30)
	sb	x6, 1(x30)
	lb x28,2(x5)
	lb x6,3(x5)
	sb	x28, 2(x30)
	sb	x6, 3(x30)
	lb x28,4(x5)
	sb	x28, 4(x30)
	li x12,716527
	call _trace
	la x30,L.13
	lw x30,0(x30)
	sw x30,-272+288(x8)
	la x30,L.14
	lw x30,0(x30)
	sw x30,-276+288(x8)
	li x12,255
	li x13,1005
	addi x14,x8,-261+288
	li x15,1
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	beq x27,x0,L.15
	li x12,1
	call ak_exit
L.15:
	li x12,256
	li x13,1005
	addi x14,x8,-262+288
	li x15,1
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	beq x27,x0,L.17
	li x12,2
	call ak_exit
L.17:
	li x12,256
	li x13,1002
	addi x14,x8,-267+288
	li x15,4
	call ak_dev_ctl
	j L.20
L.19:
	li x12,256
	li x13,1002
	la x14,L.22
	li x15,2
	call ak_dev_ctl
	li x12,255
	li x13,1001
	addi x14,x8,-260+288
	li x15,256
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	li x30,256
	beq x27,x30,L.23
	li x12,786650
	mv x13,x27 ; LOADI4
	call _trace
	li x12,256
	li x13,1002
	la x14,L.25
	li x15,20
	call ak_dev_ctl
	j L.24
L.23:
	addi x12,x8,-260+288
	mv x13,x27 ; LOADU4
	mv x14,x25 ; LOADP4
	call tokenize
	mv x27,x10 ; LOADI4
	mv x26,x0 ; LOADU4
	j L.29
L.26:
	li x30,2
	shll x30,x26,x30
	add x30,x30,x25
	lw x12,0(x30)
	call strlen
	mv x30,x10 ; LOADI4
	li x12,256
	li x13,1002
	li x29,2
	shll x29,x26,x29
	add x29,x29,x25
	lw x14,0(x29)
	mv x15,x30 ; LOADU4
	call ak_dev_ctl
	li x12,256
	mv x13,x0 ; LOADI4
	li x14,10
	call ak_dev_out
L.27:
	li x30,1
	add x26,x26,x30
L.29:
	mv x30,x27 ; LOADU4
	bltu x26,x30,L.26
L.24:
L.20:
	j L.19
L.9:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,332(x2)
	addi  x2,x2,336
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.25:
	.byte	0x45
	.byte	0x72
	.byte	0x72
	.byte	0x6f
	.byte	0x72
	.byte	0x20
	.byte	0x72
	.byte	0x65
	.byte	0x61
	.byte	0x64
	.byte	0x69
	.byte	0x6e
	.byte	0x67
	.byte	0x20
	.byte	0x6c
	.byte	0x69
	.byte	0x6e
	.byte	0x65
	.byte	0x21
	.byte	0xa
	.byte	0x0
	.align	1
L.22:
	.byte	0x3e
	.byte	0x20
	.byte	0x0
	.align	4
L.14:
	.word	0xc4ec4a3d
	.align	4
L.13:
	.word	0x417ab021
	.align 4
