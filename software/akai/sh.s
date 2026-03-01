# Compiled with lcc-sirius 4.2
# assemble with supplied as assembler
# link with supplied ld linker
	.align 4
	.text
	.align	4
emit:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x30,+(0)+16(x8)
	sb x30,+(0)+16(x8)
	li x12,855309
	lbu x13,+(0)+16(x8)
	call _trace
	li x12,256
	mv x13,x0 ; LOADI4
	lbu x14,+(0)+16(x8)
	call ak_dev_out
L.8:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.10:
	.word	L.11
	.align	1
L.39:
	.byte	0x30
	.byte	0x31
	.byte	0x32
	.byte	0x33
	.byte	0x34
	.byte	0x35
	.byte	0x36
	.byte	0x37
	.byte	0x38
	.byte	0x39
	.byte	0x41
	.byte	0x42
	.byte	0x43
	.byte	0x44
	.byte	0x45
	.byte	0x46
	.byte	0x0
	.align 4
	.text
	.align	4
miniprint:
	addi x2,x2,-128
	sw  x8,124(x2)
	addi  x8,x2,80
	sw x1,24(x2)
	sw x18,28(x2)
	sw x19,32(x2)
	sw x20,36(x2)
	sw x21,40(x2)
	sw x22,44(x2)
	sw x23,48(x2)
	sw x24,52(x2)
	sw x25,56(x2)
	sw x26,60(x2)
	sw x27,64(x2)
	sw x12,48(x8)
	sw x13,52(x8)
	sw x14,56(x8)
	sw x15,60(x8)
	sw x16,64(x8)
	sw x17,68(x8)
	mv x27,x0 ; LOADU1
	addi x25,x8,8+48
	j L.14
L.13:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,37
	bne x30,x29,L.16
	mv x24,x0 ; LOADU1
	mv x23,x0 ; LOADI1
	lw x30,+(0)+48(x8)
	li x29,1
	add x29,x30,x29
	sw x29,+(0)+48(x8)
	lbu x27,+0(x30)
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.18
	mv x10,x0 ; LOADI4
	j L.9
L.18:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	bne x30,x29,L.20
	li x23,1
	lw x30,+(0)+48(x8)
	li x29,1
	add x29,x30,x29
	sw x29,+(0)+48(x8)
	lbu x27,+0(x30)
L.20:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.22
	mv x10,x0 ; LOADI4
	j L.9
L.22:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	blt x30,x29,L.24
	li x29,57
	bgt x30,x29,L.24
	li x30,10
	shlli x29,x24,8*(4-1)
	shrli x29,x29,8*(4-1)
	imul x0,x30,x30,x29
	shlli x29,x27,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,-48
	add x30,x30,x29
	mv x24,x30 ; LOADU1
	lw x30,+(0)+48(x8)
	li x29,1
	add x29,x30,x29
	sw x29,+(0)+48(x8)
	lbu x27,+0(x30)
L.24:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.26
	mv x10,x0 ; LOADI4
	j L.9
L.26:
	shlli x22,x27,8*(4-1)
	shrli x22,x22,8*(4-1)
	li x30,99
	beq x22,x30,L.37
	li x30,100
	beq x22,x30,L.55
	bgt x22,x30,L.85
L.84:
	li x30,37
	beq x22,x30,L.83
	j L.17
L.85:
	li x30,115
	beq x22,x30,L.31
	blt x22,x30,L.17
L.86:
	li x30,120
	beq x22,x30,L.38
	j L.17
L.31:
	li x30,8
	add x30,x25,x30
	mv x25,x30 ; LOADP4
	li x29,-8
	add x30,x30,x29
	lw x21,+0(x30)
	mv x30,x21 ; LOADU4
	bne x30,x0,L.35
	la x30,L.10
	lw x21,+0(x30)
	j L.35
L.34:
	mv x30,x21 ; LOADP4
	li x29,1
	add x21,x30,x29
	lbu x12,+0(x30)
	call emit
L.35:
	lbu x30,+0(x21)
	bne x30,x0,L.34
	j L.17
L.37:
	li x30,8
	add x30,x25,x30
	mv x25,x30 ; LOADP4
	li x29,-8
	add x30,x30,x29
	lw x30,+0(x30)
	sb x30,+(-5)+48(x8)
	lbu x12,+(-5)+48(x8)
	call emit
	j L.17
L.38:
	addi x30,x8,-37+48
	la x5,L.39
	li x28,17
	copy x5,x30,x28
	mv x21,x0 ; LOADI4
	li x30,8
	add x30,x25,x30
	mv x25,x30 ; LOADP4
	li x29,-8
	add x30,x30,x29
	lw x30,+0(x30)
	mv x20,x30 ; LOADU4
	bne x20,x0,L.44
	mv x30,x21 ; LOADI4
	addi x21,x30,1
	addi x29,x8,-20+48
	add x30,x30,x29
	li x29,48
	sb x29,+0(x30)
	j L.42
L.43:
	mv x30,x21 ; LOADI4
	addi x21,x30,1
	addi x29,x8,-20+48
	add x30,x30,x29
	li x29,15
	and x29,x20,x29
	addi x28,x8,-37+48
	add x29,x29,x28
	lbu x29,+0(x29)
	sb x29,+0(x30)
	li x30,4
	shrl x20,x20,x30
L.44:
	bne x20,x0,L.43
L.42:
	mv x30,x21 ; LOADU4
	mv x19,x30 ; LOADU1
	j L.47
L.46:
	shlli x30,x23,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.50
	li x18,48
	j L.51
L.50:
	li x18,32
L.51:
	mv x30,x18 ; LOADU4
	shlli x12,x30,8*(4-1)
	shrli x12,x12,8*(4-1)
	call emit
	shlli x30,x19,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	mv x19,x30 ; LOADU1
L.47:
	shlli x30,x19,8*(4-1)
	shrli x30,x30,8*(4-1)
	shlli x29,x24,8*(4-1)
	shrli x29,x29,8*(4-1)
	blt x30,x29,L.46
	j L.53
L.52:
	addi x30,x21,-1
	mv x21,x30 ; LOADI4
	addi x29,x8,-20+48
	add x30,x30,x29
	lbu x12,+0(x30)
	call emit
L.53:
	bgt x21,x0,L.52
	j L.17
L.55:
	mv x21,x0 ; LOADI4
	li x30,8
	add x30,x25,x30
	mv x25,x30 ; LOADP4
	li x29,-8
	add x30,x30,x29
	lw x30,+0(x30)
	sw x30,+(-24)+48(x8)
	mv x19,x0 ; LOADI4
	lw x30,+(-24)+48(x8)
	bge x30,x0,L.57
	li x30,1
	sw x30,+(-28)+48(x8)
	j L.58
L.57:
	sw x0,+(-28)+48(x8)
L.58:
	lw x30,+(-28)+48(x8)
	mv x18,x30 ; LOADI1
	shlli x30,x18,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.59
	lw x30,+(-24)+48(x8)
	sub x30,x0,x30
	mv x20,x30 ; LOADU4
	j L.60
L.59:
	lw x30,+(-24)+48(x8)
	mv x20,x30 ; LOADU4
L.60:
	bne x20,x0,L.65
	mv x30,x21 ; LOADI4
	addi x21,x30,1
	addi x29,x8,-20+48
	add x30,x30,x29
	li x29,48
	sb x29,+0(x30)
	j L.63
L.64:
	mv x30,x21 ; LOADI4
	addi x21,x30,1
	addi x29,x8,-20+48
	add x30,x30,x29
	li x29,10
	udiv x0,x29,x20,x29
	li x28,48
	add x29,x29,x28
	sb x29,+0(x30)
	li x30,10
	udiv x20,x0,x20,x30
L.65:
	bne x20,x0,L.64
	shlli x29,x23,8*(4-1)
	shrai x29,x29,8*(4-1)
	beq x29,x0,L.67
	shlli x29,x18,8*(4-1)
	shrai x29,x29,8*(4-1)
	beq x29,x0,L.67
	li x12,45
	call emit
L.67:
L.63:
	shlli x30,x18,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.70
	li x30,1
	sw x30,+(-32)+48(x8)
	j L.71
L.70:
	sw x0,+(-32)+48(x8)
L.71:
	lw x30,+(-32)+48(x8)
	add x19,x21,x30
	j L.73
L.72:
	shlli x30,x23,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.76
	li x30,48
	sw x30,+(-36)+48(x8)
	j L.77
L.76:
	li x30,32
	sw x30,+(-36)+48(x8)
L.77:
	lw x30,+(-36)+48(x8)
	shlli x12,x30,8*(4-1)
	shrli x12,x12,8*(4-1)
	call emit
	addi x19,x19,1
L.73:
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	blt x19,x30,L.72
	shlli x29,x23,8*(4-1)
	shrai x29,x29,8*(4-1)
	bne x29,x0,L.81
	shlli x29,x18,8*(4-1)
	shrai x29,x29,8*(4-1)
	beq x29,x0,L.81
	li x12,45
	call emit
	j L.81
L.80:
	addi x30,x21,-1
	mv x21,x30 ; LOADI4
	addi x29,x8,-20+48
	add x30,x30,x29
	lbu x12,+0(x30)
	call emit
L.81:
	bgt x21,x0,L.80
	j L.17
L.83:
	li x12,37
	call emit
	j L.17
L.16:
	shlli x12,x26,8*(4-1)
	shrli x12,x12,8*(4-1)
	call emit
L.17:
L.14:
	lw x30,+(0)+48(x8)
	li x29,1
	add x29,x30,x29
	sw x29,+(0)+48(x8)
	lbu x30,+0(x30)
	mv x26,x30 ; LOADU1
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.13
	mv x10,x0 ; LOADI4
L.9:
	lw x1,24(x2)
	lw x18,28(x2)
	lw x19,32(x2)
	lw x20,36(x2)
	lw x21,40(x2)
	lw x22,44(x2)
	lw x23,48(x2)
	lw x24,52(x2)
	lw x25,56(x2)
	lw x26,60(x2)
	lw x27,64(x2)
	lw  x8,124(x2)
	addi  x2,x2,128
	jalr x0,0(x1)

	.align	4
tokenize:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x19,0(x2)
	sw x20,4(x2)
	sw x21,8(x2)
	sw x22,12(x2)
	sw x23,16(x2)
	sw x24,20(x2)
	sw x25,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	mv x24,x0 ; LOADI4
	add x26,x13,x12
	mv x29,x12 ; LOADU4
	beq x29,x0,L.90
	mv x29,x14 ; LOADU4
	bne x29,x0,L.88
L.90:
	mv x10,x0 ; LOADI4
	j L.87
L.88:
	mv x27,x12 ; LOADP4
	mv x25,x12 ; LOADP4
	j L.94
L.91:
	mv x23,x0 ; LOADU1
	mv x30,x24 ; LOADU4
	bltu x30,x15,L.98
	j L.93
L.97:
	li x30,1
	add x27,x27,x30
L.98:
	mv x30,x27 ; LOADU4
	mv x29,x26 ; LOADU4
	bgeu x30,x29,L.101
	lbu x30,+0(x27)
	beq x30,x0,L.101
	li x28,1
	shll x30,x30,x28
	la x28,_Ctype
	lw x28,+0(x28)
	add x30,x30,x28
	lh x30,+0(x30)
	andi x30,x30,324
	bne x30,x0,L.97
L.101:
	mv x30,x27 ; LOADU4
	mv x29,x26 ; LOADU4
	bgeu x30,x29,L.104
	lbu x30,+0(x27)
	bne x30,x0,L.102
L.104:
	j L.93
L.102:
	lbu x30,+0(x27)
	li x29,39
	beq x30,x29,L.108
	li x29,34
	bne x30,x29,L.106
L.108:
	mv x30,x27 ; LOADP4
	li x29,1
	add x27,x30,x29
	lbu x22,+0(x30)
	j L.107
L.106:
	mv x22,x0 ; LOADI4
L.107:
	mv x30,x22 ; LOADU4
	mv x23,x30 ; LOADU1
	mv x30,x24 ; LOADI4
	addi x24,x30,1
	li x29,2
	shll x30,x30,x29
	add x30,x30,x14
	sw x25,+0(x30)
	j L.110
L.109:
	lbu x30,+0(x27)
	li x29,92
	bne x30,x29,L.115
	li x30,1
	add x30,x27,x30
	mv x29,x26 ; LOADU4
	bgeu x30,x29,L.115
	li x30,1
	add x30,x27,x30
	lbu x21,+0(x30)
	shlli x30,x21,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,34
	beq x30,x29,L.120
	li x29,39
	beq x30,x29,L.120
	li x29,32
	bne x30,x29,L.117
L.120:
	li x30,1
	add x27,x27,x30
	mv x29,x25 ; LOADP4
	add x25,x29,x30
	mv x28,x27 ; LOADP4
	add x27,x28,x30
	lbu x30,+0(x28)
	sb x30,+0(x29)
	j L.110
L.117:
L.115:
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	mv x28,x27 ; LOADP4
	add x27,x28,x29
	lbu x29,+0(x28)
	sb x29,+0(x30)
L.110:
	mv x30,x27 ; LOADU4
	mv x29,x26 ; LOADU4
	bgeu x30,x29,L.122
	lbu x29,+0(x27)
	beq x29,x0,L.122
	shlli x29,x23,8*(4-1)
	shrli x29,x29,8*(4-1)
	beq x29,x0,L.123
	lbu x30,+0(x27)
	shlli x29,x23,8*(4-1)
	shrli x29,x29,8*(4-1)
	beq x30,x29,L.125
	li x20,1
	j L.126
L.125:
	mv x20,x0 ; LOADI4
L.126:
	mv x21,x20 ; LOADI4
	j L.124
L.123:
	lbu x30,+0(x27)
	li x29,1
	shll x30,x30,x29
	la x29,_Ctype
	lw x29,+0(x29)
	add x30,x30,x29
	lh x30,+0(x30)
	andi x30,x30,324
	bne x30,x0,L.127
	li x19,1
	j L.128
L.127:
	mv x19,x0 ; LOADI4
L.128:
	mv x21,x19 ; LOADI4
L.124:
	bne x21,x0,L.109
L.122:
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	sb x0,+0(x30)
	mv x30,x27 ; LOADU4
	mv x29,x26 ; LOADU4
	bgeu x30,x29,L.129
	li x30,1
	add x27,x27,x30
L.129:
L.92:
L.94:
	mv x30,x27 ; LOADU4
	mv x29,x26 ; LOADU4
	bltu x30,x29,L.91
L.93:
	mv x10,x24 ; LOADI4
L.87:
	lw x19,0(x2)
	lw x20,4(x2)
	lw x21,8(x2)
	lw x22,12(x2)
	lw x23,16(x2)
	lw x24,20(x2)
	lw x25,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.132:
	.byte	0x1b
	.byte	0x5b
	.byte	0x32
	.byte	0x4a
	.byte	0x0
	.align 4
	.text
	.align	4
cmd_clear:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	addi x30,x8,-9+16
	la x5,L.132
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
	li x12,256
	li x13,1002
	addi x14,x8,-9+16
	li x15,4
	call ak_dev_ctl
	mv x10,x0 ; LOADI4
L.131:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
getcwd:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x12,+(0)+16(x8)
	lw x13,+(4)+16(x8)
	call ak_getcwd
	sw x10,+(-8)+16(x8)
	lw x30,+(-8)+16(x8)
	beq x30,x0,L.134
	lw x10,+(-8)+16(x8)
	j L.133
L.134:
	lw x30,+(4)+16(x8)
	li x29,1
	sub x30,x30,x29
	lw x29,+(0)+16(x8)
	add x30,x30,x29
	sb x0,+0(x30)
	mv x10,x0 ; LOADI4
L.133:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.bss
	.align	1
L.137:
	.space	32
	.align 4
	.text
	.align	4
prompt:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x0,+(-8)+16(x8)
	la x12,L.137
	li x13,32
	call getcwd
	sw x10,+(-8)+16(x8)
	beq x10,x0,L.138
	j L.136
L.138:
	la x12,L.137
	call strlen
	mv x30,x10 ; LOADU4
	li x12,256
	li x13,1002
	la x14,L.137
	mv x15,x30 ; LOADU4
	call ak_dev_ctl
	lw x12,+(0)+16(x8)
	call strlen
	mv x30,x10 ; LOADU4
	li x12,256
	li x13,1002
	lw x14,+(0)+16(x8)
	mv x15,x30 ; LOADU4
	call ak_dev_ctl
L.136:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.bss
	.align	1
L.141:
	.space	32
	.align 4
	.text
	.align	4
cmd_pwd:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x0,+(-8)+16(x8)
	la x12,L.141
	li x13,32
	call getcwd
	sw x10,+(-8)+16(x8)
	beq x10,x0,L.142
	lw x10,+(-8)+16(x8)
	j L.140
L.142:
	la x12,L.141
	call strlen
	mv x30,x10 ; LOADU4
	li x12,256
	li x13,1002
	la x14,L.141
	mv x15,x30 ; LOADU4
	call ak_dev_ctl
	li x12,10
	call emit
	mv x10,x0 ; LOADI4
L.140:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.bss
	.align	1
L.145:
	.space	256
	.align	1
L.146:
	.space	128
	.align 4
	.text
	.align	4
cmd_ls:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,+(0)+16(x8)
	li x29,2
	bge x30,x29,L.149
	la x26,L.147
	j L.150
L.149:
	lw x30,+(4)+16(x8)
	li x29,4
	add x30,x30,x29
	lw x26,+0(x30)
L.150:
	sw x26,+(-8)+16(x8)
	la x12,L.145
	lw x13,+(-8)+16(x8)
	mv x14,x0 ; LOADI4
	call ak_opendir
	mv x27,x10 ; LOADI4
	beq x27,x0,L.151
	mv x10,x27 ; LOADI4
	j L.144
L.151:
L.153:
	la x12,L.145
	la x13,L.146
	mv x14,x0 ; LOADI4
	call ak_readdir
	mv x27,x10 ; LOADI4
	beq x27,x0,L.156
	mv x10,x27 ; LOADI4
	j L.144
L.156:
	la x12,L.146+16
	call strlen
	mv x30,x10 ; LOADU4
	li x12,256
	li x13,1002
	la x14,L.146+16
	mv x15,x30 ; LOADU4
	call ak_dev_ctl
	li x12,256
	li x13,1002
	la x14,L.160
	li x15,1
	call ak_dev_ctl
L.154:
	la x30,L.146+16
	lbu x30,+0(x30)
	bne x30,x0,L.153
	la x12,L.145
	mv x13,x0 ; LOADI4
	call ak_closedir
	mv x27,x10 ; LOADI4
	mv x10,x27 ; LOADI4
L.144:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
builtins:
	.word	L.162
	.word	cmd_clear
	.word	L.163
	.word	cmd_pwd
	.word	L.164
	.word	cmd_ls
	.word	0x0
	.word	0x0
	.align 4
	.text
	.align	4
aksh_exec:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x23,28(x2)
	sw x24,32(x2)
	sw x25,36(x2)
	sw x26,40(x2)
	sw x27,44(x2)
	mv x27,x12
	mv x26,x13
	mv x25,x0 ; LOADI4
	mv x24,x0 ; LOADI4
	j L.167
L.166:
	lw x12,+0(x26)
	li x30,3
	shll x30,x25,x30
	la x29,builtins
	add x30,x30,x29
	lw x13,+0(x30)
	call strcmp
	bne x10,x0,L.169
	mv x12,x27 ; LOADI4
	mv x13,x26 ; LOADP4
	li x30,3
	shll x30,x25,x30
	la x29,builtins+4
	add x30,x30,x29
	lw x30,+0(x30)
	jalr x1, 0(x30)
	mv x23,x10 ; LOADI4
	li x24,1
	j L.168
L.169:
	addi x25,x25,1
L.167:
	li x30,3
	shll x30,x25,x30
	la x29,builtins
	add x30,x30,x29
	lw x30,+0(x30)
	bne x30,x0,L.166
L.168:
	bne x24,x0,L.172
	li x12,256
	li x13,1002
	la x14,L.174
	li x15,16
	call ak_dev_ctl
L.172:
	mv x10,x23 ; LOADI4
L.165:
	lw x1,24(x2)
	lw x23,28(x2)
	lw x24,32(x2)
	lw x25,36(x2)
	lw x26,40(x2)
	lw x27,44(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.176:
	.byte	0x7
	.align	1
L.177:
	.byte	0x1
	.globl main
	.align 4
	.text
	.align	4
main:
	addi x2,x2,-384
	sw  x8,380(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x27,28(x2)
	addi x30,x8,-325+336
	la x5,L.176
	lb x28,0(x5)
	sb	x28, 0(x30)
	addi x30,x8,-326+336
	la x5,L.177
	lb x28,0(x5)
	sb	x28, 0(x30)
	li x12,255
	li x13,1005
	addi x14,x8,-325+336
	li x15,1
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	beq x27,x0,L.178
	li x12,1
	call ak_exit
L.178:
	li x12,256
	li x13,1005
	addi x14,x8,-326+336
	li x15,1
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	beq x27,x0,L.180
	li x12,2
	call ak_exit
L.180:
	mv x12,x0 ; LOADI4
	mv x13,x0 ; LOADP4
	call cmd_clear
	j L.183
L.182:
	la x12,L.185
	call prompt
	li x12,255
	li x13,1001
	addi x14,x8,-260+336
	li x15,256
	call ak_dev_ctl
	mv x27,x10 ; LOADI4
	bge x27,x0,L.186
	li x12,786650
	mv x13,x27 ; LOADI4
	call _trace
	li x12,256
	li x13,1002
	la x14,L.188
	li x15,20
	call ak_dev_ctl
	j L.187
L.186:
	addi x12,x8,-260+336
	mv x13,x27 ; LOADU4
	addi x14,x8,-324+336
	li x15,16
	call tokenize
	mv x27,x10 ; LOADI4
	ble x27,x0,L.189
	mv x12,x27 ; LOADI4
	addi x13,x8,-324+336
	call aksh_exec
L.189:
L.187:
L.183:
	j L.182
L.175:
	lw x1,24(x2)
	lw x27,28(x2)
	lw  x8,380(x2)
	addi  x2,x2,384
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.188:
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
L.185:
	.byte	0x3e
	.byte	0x20
	.byte	0x0
	.align	1
L.174:
	.byte	0x55
	.byte	0x6e
	.byte	0x6b
	.byte	0x6e
	.byte	0x6f
	.byte	0x77
	.byte	0x6e
	.byte	0x20
	.byte	0x63
	.byte	0x6f
	.byte	0x6d
	.byte	0x6d
	.byte	0x61
	.byte	0x6e
	.byte	0x64
	.byte	0xa
	.byte	0x0
	.align	1
L.164:
	.byte	0x6c
	.byte	0x73
	.byte	0x0
	.align	1
L.163:
	.byte	0x70
	.byte	0x77
	.byte	0x64
	.byte	0x0
	.align	1
L.162:
	.byte	0x63
	.byte	0x6c
	.byte	0x65
	.byte	0x61
	.byte	0x72
	.byte	0x0
	.align	1
L.160:
	.byte	0xa
	.byte	0x0
	.align	1
L.147:
	.byte	0x2e
	.byte	0x0
	.align	1
L.11:
	.byte	0x28
	.byte	0x4e
	.byte	0x55
	.byte	0x4c
	.byte	0x4c
	.byte	0x29
	.byte	0x0
	.align 4
