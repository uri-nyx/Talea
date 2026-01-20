	.align 4
	.text
	.globl ttty_init
	.align	4
ttty_init:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x25,0(x2)
	sw x26,4(x2)
	sw x27,8(x2)
	lw x27,28+16(x8)
	shlli x30,x17,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,8
	ble x30,x29,L.7
	mv x10,x0 ; LOADI4
	j L.6
L.7:
	sw x13,0(x12)
	li x30,12
	add x30,x12,x30
	andi x29,x14,-129
	sw x29,0(x30)
	andi x30,x14,128
	beq x30,x0,L.10
	li x25,1
	j L.11
L.10:
	mv x25,x0 ; LOADI4
L.11:
	li x30,20
	add x30,x12,x30
	mv x29,x25 ; LOADI1
	sb x29,0(x30)
	li x30,20
	add x30,x12,x30
	lb x30,0(x30)
	beq x30,x0,L.12
	li x30,48
	add x30,x12,x30
	sw x0,0(x30)
	li x30,52
	add x30,x12,x30
	sb x0,0(x30)
	li x30,44
	add x30,x12,x30
	sw x0,0(x30)
	li x30,24
	add x30,x12,x30
	sw x0,0(x30)
L.12:
	li x30,4
	add x30,x12,x30
	sb x15,0(x30)
	li x30,5
	add x30,x12,x30
	sb x16,0(x30)
	li x30,8
	add x30,x12,x30
	sb x17,0(x30)
	li x30,6
	add x30,x12,x30
	sb x0,0(x30)
	li x30,7
	add x30,x12,x30
	sb x0,0(x30)
	li x30,16
	add x30,x12,x30
	lw x29,24+16(x8)
	sw x29,0(x30)
	li x30,68
	add x30,x12,x30
	lw x29,32+16(x8)
	sw x29,0(x30)
	li x30,72
	add x30,x12,x30
	lw x29,36+16(x8)
	sw x29,0(x30)
	li x30,9
	add x30,x12,x30
	sb x0,0(x30)
	li x30,64
	add x30,x12,x30
	li x29,8
	sb x29,0(x30)
	mv x26,x0 ; LOADU1
	j L.17
L.14:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,56
	add x29,x12,x29
	add x29,x30,x29
	add x30,x30,x27
	lbu x30,0(x30)
	sb x30,0(x29)
L.15:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	mv x26,x30 ; LOADU1
L.17:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	shlli x29,x17,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,-1
	blt x30,x29,L.14
	li x10,1
L.6:
	lw x25,0(x2)
	lw x26,4(x2)
	lw x27,8(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl ttty_mux_subscribe
	.align	4
ttty_mux_subscribe:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,36
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,4
	blt x30,x29,L.19
	mv x10,x0 ; LOADI4
	j L.18
L.19:
	li x30,36
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,3
	shll x30,x30,x29
	add x30,x30,x12
	sw x13,0(x30)
	li x30,36
	add x30,x12,x30
	lbu x29,0(x30)
	shlli x28,x29,8*(4-1)
	shrli x28,x28,8*(4-1)
	addi x28,x28,1
	sb x28,0(x30)
	shlli x30,x29,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,3
	shll x30,x30,x29
	add x30,x30,x12
	li x29,4
	add x30,x30,x29
	sw x14,0(x30)
	li x10,1
L.18:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl ttty_mux_set_priority
	.align	4
ttty_mux_set_priority:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,32
	add x30,x12,x30
	sw x13,0(x30)
L.21:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align	4
ttty_mux_check_priority:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x13,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,36
	add x29,x12,x29
	lbu x29,0(x29)
	bge x30,x29,L.23
	shlli x30,x13,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,3
	shll x30,x30,x29
	add x30,x30,x12
	li x29,4
	add x30,x30,x29
	lw x30,0(x30)
	li x29,32
	add x29,x12,x29
	lw x29,0(x29)
	blt x30,x29,L.26
	li x27,1
	j L.27
L.26:
	mv x27,x0 ; LOADI4
L.27:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
	j L.22
L.23:
	mv x10,x0 ; LOADI4
L.22:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align	4
ttty_update_cursor_pos:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x27,28(x2)
	mv x27,x12
	mv x12,x27 ; LOADP4
	li x30,6
	add x30,x27,x30
	lbu x13,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x14,0(x30)
	li x30,72
	add x30,x27,x30
	lw x31,0(x30)
	jalr x1, 0(x31)
L.28:
	lw x1,24(x2)
	lw x27,28(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align	4
ttty_set_xy:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	mv x26,x13
	mv x25,x14
	bge x26,x0,L.30
	mv x26,x0 ; LOADI4
L.30:
	li x30,4
	add x30,x27,x30
	lbu x30,0(x30)
	blt x26,x30,L.32
	li x30,4
	add x30,x27,x30
	lbu x30,0(x30)
	addi x26,x30,-1
L.32:
	bge x25,x0,L.34
	mv x25,x0 ; LOADI4
L.34:
	li x30,5
	add x30,x27,x30
	lbu x30,0(x30)
	blt x25,x30,L.36
	li x30,5
	add x30,x27,x30
	lbu x30,0(x30)
	addi x25,x30,-1
L.36:
	li x30,6
	add x30,x27,x30
	mv x29,x26 ; LOADU4
	sb x29,0(x30)
	li x30,7
	add x30,x27,x30
	mv x29,x25 ; LOADU4
	sb x29,0(x30)
	mv x12,x27 ; LOADP4
	call ttty_update_cursor_pos
L.29:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align	4
ttty_scroll_up:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x23,0(x2)
	sw x24,4(x2)
	sw x25,8(x2)
	sw x26,12(x2)
	sw x27,16(x2)
	li x30,16
	add x30,x12,x30
	lw x30,0(x30)
	beq x30,x0,L.39
	li x30,16
	add x30,x12,x30
	lw x25,0(x30)
	li x30,4
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,8
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x24,x30,x29
	li x30,4
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	li x29,8
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x23,x30,x29
	mv x26,x0 ; LOADI4
	j L.44
L.41:
	add x30,x26,x25
	add x29,x26,x24
	add x29,x29,x25
	lbu x29,0(x29)
	sb x29,0(x30)
L.42:
	addi x26,x26,1
L.44:
	sub x30,x23,x24
	blt x26,x30,L.41
	sub x26,x23,x24
	j L.48
L.45:
	add x30,x26,x25
	li x29,32
	sb x29,0(x30)
	li x27,1
	j L.52
L.49:
	add x30,x26,x27
	add x30,x30,x25
	addi x29,x27,-1
	li x28,56
	add x28,x12,x28
	add x29,x29,x28
	lbu x29,0(x29)
	sb x29,0(x30)
L.50:
	addi x27,x27,1
L.52:
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	blt x27,x30,L.49
L.46:
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	add x26,x26,x30
L.48:
	blt x26,x23,L.45
L.39:
	li x30,7
	add x30,x12,x30
	li x29,5
	add x29,x12,x29
	lbu x29,0(x29)
	addi x29,x29,-1
	sb x29,0(x30)
L.38:
	lw x23,0(x2)
	lw x24,4(x2)
	lw x25,8(x2)
	lw x26,12(x2)
	lw x27,16(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
ttty_handle_tab:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x22,28(x2)
	sw x23,32(x2)
	sw x24,36(x2)
	sw x25,40(x2)
	sw x26,44(x2)
	sw x27,48(x2)
	mv x27,x12
	li x30,64
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,6
	add x29,x27,x29
	lbu x29,0(x29)
	idiv x0,x29,x29,x30
	sub x26,x30,x29
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	add x30,x30,x26
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.54
	li x30,6
	add x30,x27,x30
	sb x0,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x29,0(x30)
	addi x29,x29,1
	sb x29,0(x30)
	j L.55
L.54:
	li x30,16
	add x30,x27,x30
	lw x23,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	li x29,6
	add x29,x27,x29
	lbu x29,0(x29)
	add x30,x30,x29
	li x29,8
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	sw x30,-8+16(x8)
	lw x30,-8+16(x8)
	li x29,8
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x29,x26,x29
	add x22,x30,x29
	lw x24,-8+16(x8)
	j L.59
L.56:
	add x30,x24,x23
	li x29,32
	sb x29,0(x30)
	li x25,1
	j L.63
L.60:
	add x30,x24,x25
	add x30,x30,x23
	li x29,1
	sub x29,x25,x29
	li x28,56
	add x28,x27,x28
	add x29,x29,x28
	lbu x29,0(x29)
	sb x29,0(x30)
L.61:
	li x30,1
	add x25,x25,x30
L.63:
	li x30,8
	add x30,x27,x30
	lbu x30,0(x30)
	bltu x25,x30,L.60
L.57:
	li x30,8
	add x30,x27,x30
	lbu x30,0(x30)
	add x24,x24,x30
L.59:
	bltu x24,x22,L.56
	li x30,6
	add x30,x27,x30
	lbu x29,0(x30)
	add x29,x29,x26
	sb x29,0(x30)
L.55:
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.64
	mv x12,x27 ; LOADP4
	call ttty_scroll_up
L.64:
L.53:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x23,32(x2)
	lw x24,36(x2)
	lw x25,40(x2)
	lw x26,44(x2)
	lw x27,48(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl ttty_emit
	.align	4
ttty_emit:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	mv x27,x12
	sw x13,20(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	mv x29,x27 ; LOADU4
	beq x29,x0,L.69
	li x29,68
	add x29,x27,x29
	lw x29,0(x29)
	bne x29,x0,L.67
L.69:
	j L.66
L.67:
	li x30,12
	add x30,x27,x30
	lw x30,0(x30)
	bne x30,x0,L.70
	mv x12,x27 ; LOADP4
	lbu x13,4+16(x8)
	li x30,68
	add x30,x27,x30
	lw x31,0(x30)
	jalr x1, 0(x31)
	j L.66
L.70:
	lbu x26,4+16(x8)
	li x30,8
	blt x26,x30,L.73
	li x30,13
	bgt x26,x30,L.73
	li x30,2
	shll x30,x26,x30
	la x29,L.86-32
	add x30,x30,x29
	lw x30,0(x30)
	jalr x0,0(x30)
	.align 4
	.data
	.align	4
L.86:
	.word	L.81
	.word	L.84
	.word	L.75
	.word	L.73
	.word	L.85
	.word	L.79
	.align 4
	.text
L.75:
	li x30,12
	add x30,x27,x30
	lw x30,0(x30)
	li x29,1
	bne x30,x29,L.76
	li x30,6
	add x30,x27,x30
	sb x0,0(x30)
L.76:
	li x30,7
	add x30,x27,x30
	lbu x29,0(x30)
	addi x29,x29,1
	sb x29,0(x30)
	j L.78
L.79:
	li x30,6
	add x30,x27,x30
	sb x0,0(x30)
	j L.80
L.81:
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	ble x30,x0,L.82
	li x30,6
	add x30,x27,x30
	lbu x29,0(x30)
	addi x29,x29,-1
	sb x29,0(x30)
L.82:
	li x30,9
	add x30,x27,x30
	li x29,1
	sb x29,0(x30)
	j L.80
L.84:
	mv x12,x27 ; LOADP4
	call ttty_handle_tab
	j L.80
L.85:
	mv x12,x27 ; LOADP4
	call ttty_clear
	j L.80
L.73:
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.88
	li x30,6
	add x30,x27,x30
	sb x0,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x29,0(x30)
	addi x29,x29,1
	sb x29,0(x30)
L.88:
L.78:
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.90
	mv x12,x27 ; LOADP4
	call ttty_scroll_up
L.90:
	lbu x30,4+16(x8)
	li x29,32
	blt x30,x29,L.92
	li x30,68
	add x30,x27,x30
	lw x30,0(x30)
	beq x30,x0,L.94
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	bge x30,x29,L.94
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x27,x29
	lbu x29,0(x29)
	bge x30,x29,L.94
	mv x12,x27 ; LOADP4
	lbu x13,4+16(x8)
	li x30,68
	add x30,x27,x30
	lw x31,0(x30)
	jalr x1, 0(x31)
L.94:
	li x30,6
	add x30,x27,x30
	lbu x29,0(x30)
	addi x29,x29,1
	sb x29,0(x30)
L.92:
L.80:
	mv x12,x27 ; LOADP4
	call ttty_update_cursor_pos
L.66:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align	4
ttty_execute_ansi:
	addi x2,x2,-144
	sw  x8,140(x2)
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
	mv x27,x12
	sw x13,68(x8)
	li x30,16
	add x30,x27,x30
	lw x26,0(x30)
	lw x30,4+64(x8)
	li x29,4
	add x25,x30,x29
	lw x30,0(x25)
	ble x30,x0,L.98
	li x24,1
	j L.99
L.98:
	mv x24,x0 ; LOADI4
L.99:
	sub x30,x30,x24
	sw x30,-12+64(x8)
	li x30,4
	add x30,x25,x30
	lw x30,0(x30)
	ble x30,x0,L.101
	li x23,1
	j L.102
L.101:
	mv x23,x0 ; LOADI4
L.102:
	sub x30,x30,x23
	sw x30,-16+64(x8)
	li x30,8
	add x30,x25,x30
	lw x30,0(x30)
	ble x30,x0,L.104
	li x22,1
	j L.105
L.104:
	mv x22,x0 ; LOADI4
L.105:
	sub x30,x30,x22
	sw x30,-20+64(x8)
	li x30,12
	add x30,x25,x30
	lw x30,0(x30)
	ble x30,x0,L.107
	li x21,1
	j L.108
L.107:
	mv x21,x0 ; LOADI4
L.108:
	sub x30,x30,x21
	sw x30,-24+64(x8)
	lw x30,0(x25)
	ble x30,x0,L.110
	lw x20,0(x25)
	j L.111
L.110:
	li x20,1
L.111:
	sw x20,-8+64(x8)
	lw x30,4+64(x8)
	lw x19,0(x30)
	li x30,65
	blt x19,x30,L.113
	li x30,75
	bgt x19,x30,L.152
	li x30,2
	shll x30,x19,x30
	la x29,L.153-260
	add x30,x30,x29
	lw x30,0(x30)
	jalr x0,0(x30)
	.align 4
	.data
	.align	4
L.153:
	.word	L.115
	.word	L.116
	.word	L.117
	.word	L.118
	.word	L.119
	.word	L.120
	.word	L.121
	.word	L.122
	.word	L.113
	.word	L.123
	.word	L.138
	.align 4
	.text
L.152:
	li x30,109
	beq x19,x30,L.113
	j L.113
L.115:
	mv x12,x27 ; LOADP4
	li x30,6
	add x30,x27,x30
	lbu x13,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	sub x14,x30,x29
	call ttty_set_xy
	j L.113
L.116:
	mv x12,x27 ; LOADP4
	li x30,6
	add x30,x27,x30
	lbu x13,0(x30)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	add x14,x30,x29
	call ttty_set_xy
	j L.113
L.117:
	mv x12,x27 ; LOADP4
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	add x13,x30,x29
	li x30,7
	add x30,x27,x30
	lbu x14,0(x30)
	call ttty_set_xy
	j L.113
L.118:
	mv x12,x27 ; LOADP4
	li x30,6
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	sub x13,x30,x29
	li x30,7
	add x30,x27,x30
	lbu x14,0(x30)
	call ttty_set_xy
	j L.113
L.119:
	mv x12,x27 ; LOADP4
	mv x13,x0 ; LOADI4
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	add x14,x30,x29
	call ttty_set_xy
	j L.113
L.120:
	mv x12,x27 ; LOADP4
	mv x13,x0 ; LOADI4
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	lw x29,-8+64(x8)
	sub x14,x30,x29
	call ttty_set_xy
	j L.113
L.121:
	mv x12,x27 ; LOADP4
	lw x13,-12+64(x8)
	li x30,7
	add x30,x27,x30
	lbu x14,0(x30)
	call ttty_set_xy
	j L.113
L.122:
	mv x12,x27 ; LOADP4
	lw x13,-16+64(x8)
	lw x14,-12+64(x8)
	call ttty_set_xy
	j L.113
L.123:
	sw x0,-32+64(x8)
	mv x18,x0 ; LOADI4
	li x30,4
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	sw x30,-28+64(x8)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	li x29,6
	add x29,x27,x29
	lbu x29,0(x29)
	add x30,x30,x29
	sw x30,-36+64(x8)
	lw x30,0(x25)
	bne x30,x0,L.124
	lw x30,-36+64(x8)
	sw x30,-32+64(x8)
	j L.125
L.124:
	lw x30,0(x25)
	li x29,1
	bne x30,x29,L.126
	lw x30,-36+64(x8)
	addi x30,x30,1
	sw x30,-28+64(x8)
	j L.127
L.126:
	lw x30,0(x25)
	li x29,2
	bne x30,x29,L.128
	mv x12,x27 ; LOADP4
	call ttty_clear
	j L.113
L.128:
L.127:
L.125:
	lw x18,-32+64(x8)
	j L.133
L.130:
	li x30,8
	add x30,x27,x30
	lbu x30,0(x30)
	imul x0,x30,x18,x30
	sw x30,-44+64(x8)
	sw x0,-40+64(x8)
	lw x30,-44+64(x8)
	add x30,x30,x26
	li x29,32
	sb x29,0(x30)
	li x30,1
	sw x30,-40+64(x8)
	j L.137
L.134:
	lw x30,-40+64(x8)
	lw x29,-44+64(x8)
	add x29,x29,x30
	add x29,x29,x26
	addi x30,x30,-1
	li x28,56
	add x28,x27,x28
	add x30,x30,x28
	lbu x30,0(x30)
	sb x30,0(x29)
L.135:
	lw x30,-40+64(x8)
	addi x30,x30,1
	sw x30,-40+64(x8)
L.137:
	lw x30,-40+64(x8)
	li x29,8
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.134
L.131:
	addi x18,x18,1
L.133:
	lw x30,-28+64(x8)
	blt x18,x30,L.130
	j L.113
L.138:
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	sw x30,-36+64(x8)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	addi x30,x30,1
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	sw x30,-40+64(x8)
	li x30,7
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,4
	add x29,x27,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	li x29,6
	add x29,x27,x29
	lbu x29,0(x29)
	add x30,x30,x29
	sw x30,-44+64(x8)
	mv x18,x0 ; LOADI4
	lw x30,-36+64(x8)
	sw x30,-32+64(x8)
	lw x30,-40+64(x8)
	sw x30,-28+64(x8)
	lw x30,0(x25)
	bne x30,x0,L.139
	lw x30,-44+64(x8)
	sw x30,-32+64(x8)
	j L.140
L.139:
	lw x30,0(x25)
	li x29,1
	bne x30,x29,L.141
	lw x30,-44+64(x8)
	addi x30,x30,1
	sw x30,-28+64(x8)
L.141:
L.140:
	lw x18,-32+64(x8)
	j L.146
L.143:
	li x30,8
	add x30,x27,x30
	lbu x30,0(x30)
	imul x0,x30,x18,x30
	sw x30,-52+64(x8)
	sw x0,-48+64(x8)
	lw x30,-52+64(x8)
	add x30,x30,x26
	li x29,32
	sb x29,0(x30)
	li x30,1
	sw x30,-48+64(x8)
	j L.150
L.147:
	lw x30,-48+64(x8)
	lw x29,-52+64(x8)
	add x29,x29,x30
	add x29,x29,x26
	addi x30,x30,-1
	li x28,56
	add x28,x27,x28
	add x30,x30,x28
	lbu x30,0(x30)
	sb x30,0(x29)
L.148:
	lw x30,-48+64(x8)
	addi x30,x30,1
	sw x30,-48+64(x8)
L.150:
	lw x30,-48+64(x8)
	li x29,8
	add x29,x27,x29
	lbu x29,0(x29)
	blt x30,x29,L.147
L.144:
	addi x18,x18,1
L.146:
	lw x30,-28+64(x8)
	blt x18,x30,L.143
L.113:
L.96:
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
	lw  x8,140(x2)
	addi  x2,x2,144
	jalr x0,0(x1)

	.globl ttty_putc_ansi
	.align	4
ttty_putc_ansi:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,32(x8)
	sw x13,36(x8)
	lw x30,4+32(x8)
	sb x30,4+32(x8)
	lw x30,0+32(x8)
	li x29,12
	add x30,x30,x29
	lw x30,0(x30)
	bne x30,x0,L.156
	lw x12,0+32(x8)
	lbu x13,4+32(x8)
	call ttty_emit
	j L.155
L.156:
	lw x30,0+32(x8)
	li x29,24
	add x12,x30,x29
	lbu x13,4+32(x8)
	addi x14,x8,-32+32
	call ttty_ansi_parse
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.158
	lw x30,-32+32(x8)
	bne x30,x0,L.160
	j L.155
L.160:
	lw x30,-32+32(x8)
	li x29,1
	bne x30,x29,L.162
	lw x12,0+32(x8)
	lbu x13,4+32(x8)
	call ttty_emit
	j L.163
L.162:
	lw x12,0+32(x8)
	addi x13,x8,-32+32
	call ttty_execute_ansi
L.163:
L.158:
L.155:
	lw x1,24(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl ttty_putc
	.align	4
ttty_putc:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	lw x30,0+16(x8)
	li x29,20
	add x30,x30,x29
	lb x30,0(x30)
	beq x30,x0,L.165
	lw x12,0+16(x8)
	lbu x13,4+16(x8)
	call ttty_putc_ansi
	j L.166
L.165:
	lw x12,0+16(x8)
	lbu x13,4+16(x8)
	call ttty_emit
L.166:
L.164:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl ttty_mux_putc
	.align	4
ttty_mux_putc:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	mv x26,x13
	mv x25,x0 ; LOADI4
	j L.171
L.168:
	li x30,32
	add x30,x27,x30
	lw x30,0(x30)
	li x29,3
	shll x29,x25,x29
	add x29,x29,x27
	li x28,4
	add x29,x29,x28
	lw x29,0(x29)
	blt x30,x29,L.172
	li x30,3
	shll x30,x25,x30
	add x30,x30,x27
	lw x12,0(x30)
	shlli x13,x26,8*(4-1)
	shrli x13,x13,8*(4-1)
	call ttty_putc
L.172:
L.169:
	addi x25,x25,1
L.171:
	li x30,36
	add x30,x27,x30
	lbu x30,0(x30)
	blt x25,x30,L.168
L.167:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl ttty_puts
	.align	4
ttty_puts:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	mv x27,x12
	mv x26,x13
	j L.176
L.175:
	mv x12,x27 ; LOADP4
	mv x30,x26 ; LOADP4
	li x29,1
	add x26,x30,x29
	lbu x13,0(x30)
	call ttty_putc
L.176:
	lbu x30,0(x26)
	bne x30,x0,L.175
L.174:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl ttty_mux_puts
	.align	4
ttty_mux_puts:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	mv x26,x13
	mv x25,x0 ; LOADI4
	j L.182
L.179:
	li x30,32
	add x30,x27,x30
	lw x30,0(x30)
	li x29,3
	shll x29,x25,x29
	add x29,x29,x27
	li x28,4
	add x29,x29,x28
	lw x29,0(x29)
	blt x30,x29,L.183
	li x30,3
	shll x30,x25,x30
	add x30,x30,x27
	lw x12,0(x30)
	mv x13,x26 ; LOADP4
	call ttty_puts
L.183:
L.180:
	addi x25,x25,1
L.182:
	li x30,36
	add x30,x27,x30
	lbu x30,0(x30)
	blt x25,x30,L.179
L.178:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl ttty_clear
	.align	4
ttty_clear:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x25,0(x2)
	sw x26,4(x2)
	sw x27,8(x2)
	mv x30,x12 ; LOADU4
	beq x30,x0,L.188
	li x30,12
	add x30,x12,x30
	lw x30,0(x30)
	bne x30,x0,L.186
L.188:
	j L.185
L.186:
	li x30,6
	add x30,x12,x30
	sb x0,0(x30)
	li x30,7
	add x30,x12,x30
	sb x0,0(x30)
	li x30,12
	add x30,x12,x30
	lw x30,0(x30)
	li x29,1
	bne x30,x29,L.189
	li x30,16
	add x30,x12,x30
	lw x30,0(x30)
	beq x30,x0,L.189
	li x30,16
	add x30,x12,x30
	lw x25,0(x30)
	mv x26,x0 ; LOADI4
	j L.194
L.191:
	add x30,x26,x25
	li x29,32
	sb x29,0(x30)
	li x27,1
	j L.198
L.195:
	add x30,x26,x27
	add x30,x30,x25
	addi x29,x27,-1
	li x28,56
	add x28,x12,x28
	add x29,x29,x28
	lbu x29,0(x29)
	sb x29,0(x30)
L.196:
	addi x27,x27,1
L.198:
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	blt x27,x30,L.195
L.192:
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	add x26,x26,x30
L.194:
	li x30,4
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,5
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	li x29,8
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	blt x26,x30,L.191
L.189:
L.185:
	lw x25,0(x2)
	lw x26,4(x2)
	lw x27,8(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl ttty_mux_clear
	.align	4
ttty_mux_clear:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	mv x27,x12
	mv x26,x0 ; LOADI4
	j L.203
L.200:
	li x30,32
	add x30,x27,x30
	lw x30,0(x30)
	li x29,3
	shll x29,x26,x29
	add x29,x29,x27
	li x28,4
	add x29,x29,x28
	lw x29,0(x29)
	blt x30,x29,L.204
	li x30,3
	shll x30,x26,x30
	add x30,x30,x27
	lw x12,0(x30)
	call ttty_clear
L.204:
L.201:
	addi x26,x26,1
L.203:
	li x30,36
	add x30,x27,x30
	lbu x30,0(x30)
	blt x26,x30,L.200
L.199:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align	4
ansi_type:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x13,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.207
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	ori x10,x30,256
	j L.206
L.207:
	shlli x10,x12,8*(4-1)
	shrli x10,x10,8*(4-1)
L.206:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl ttty_ansi_parse
	.align	4
ttty_ansi_parse:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	sw x13,20(x8)
	sw x14,24(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	mv x26,x0 ; LOADI4
	lw x30,8+16(x8)
	sw x0,0(x30)
	lw x25,0(x27)
	beq x25,x0,L.213
	li x30,1
	beq x25,x30,L.216
	li x30,2
	beq x25,x30,L.223
	j L.210
L.213:
	lbu x30,4+16(x8)
	li x29,27
	bne x30,x29,L.214
	li x30,1
	sw x30,0(x27)
	mv x10,x0 ; LOADI4
	j L.209
L.214:
	lw x30,8+16(x8)
	li x29,1
	sw x29,0(x30)
	lw x30,8+16(x8)
	li x29,24
	add x30,x30,x29
	lbu x29,4+16(x8)
	sb x29,0(x30)
	li x10,1
	j L.209
L.216:
	lbu x30,4+16(x8)
	li x29,91
	bne x30,x29,L.217
	li x30,2
	sw x30,0(x27)
	li x30,20
	add x30,x27,x30
	sw x0,0(x30)
	li x30,24
	add x30,x27,x30
	sw x0,0(x30)
	li x30,28
	add x30,x27,x30
	sb x0,0(x30)
	sw x0,-8+16(x8)
	j L.222
L.219:
	lw x30,-8+16(x8)
	li x29,2
	shll x30,x30,x29
	li x29,4
	add x29,x27,x29
	add x30,x30,x29
	sw x0,0(x30)
L.220:
	lw x30,-8+16(x8)
	li x29,1
	add x30,x30,x29
	sw x30,-8+16(x8)
L.222:
	lw x30,-8+16(x8)
	li x29,4
	bltu x30,x29,L.219
	mv x10,x0 ; LOADI4
	j L.209
L.217:
	sw x0,0(x27)
	mv x10,x0 ; LOADI4
	j L.209
L.223:
	lbu x30,4+16(x8)
	li x29,63
	bne x30,x29,L.224
	li x30,28
	add x30,x27,x30
	li x29,1
	sb x29,0(x30)
	mv x10,x0 ; LOADI4
	j L.209
L.224:
	lbu x30,4+16(x8)
	li x29,48
	blt x30,x29,L.226
	li x29,57
	bgt x30,x29,L.226
	li x30,24
	add x30,x27,x30
	li x29,10
	lw x28,0(x30)
	imul x0,x29,x29,x28
	lbu x28,4+16(x8)
	addi x28,x28,-48
	add x29,x29,x28
	sw x29,0(x30)
	mv x10,x0 ; LOADI4
	j L.209
L.226:
	lbu x30,4+16(x8)
	li x29,59
	bne x30,x29,L.228
	li x30,20
	add x30,x27,x30
	lw x29,0(x30)
	addi x28,x29,1
	sw x28,0(x30)
	li x30,2
	shll x30,x29,x30
	li x29,4
	add x29,x27,x29
	add x30,x30,x29
	li x29,24
	add x29,x27,x29
	lw x29,0(x29)
	sw x29,0(x30)
	li x30,24
	add x30,x27,x30
	sw x0,0(x30)
	mv x10,x0 ; LOADI4
	j L.209
L.228:
	li x30,20
	add x30,x27,x30
	lw x29,0(x30)
	addi x28,x29,1
	sw x28,0(x30)
	li x30,2
	shll x30,x29,x30
	li x29,4
	add x29,x27,x29
	add x30,x30,x29
	li x29,24
	add x29,x27,x29
	lw x29,0(x29)
	sw x29,0(x30)
	li x30,20
	lw x29,8+16(x8)
	add x29,x29,x30
	add x30,x27,x30
	lw x30,0(x30)
	sw x30,0(x29)
	mv x26,x0 ; LOADI4
	j L.233
L.230:
	li x30,2
	shll x30,x26,x30
	li x29,4
	lw x28,8+16(x8)
	add x28,x28,x29
	add x28,x30,x28
	add x29,x27,x29
	add x30,x30,x29
	lw x30,0(x30)
	sw x30,0(x28)
L.231:
	addi x26,x26,1
L.233:
	li x30,20
	add x30,x27,x30
	lw x30,0(x30)
	blt x26,x30,L.230
	lbu x12,4+16(x8)
	li x30,28
	add x30,x27,x30
	lb x13,0(x30)
	call ansi_type
	lw x29,8+16(x8)
	sw x10,0(x29)
	sw x0,0(x27)
	li x10,1
	j L.209
L.210:
	mv x10,x0 ; LOADI4
L.209:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.235:
	.word	L.236
	.align	1
L.263:
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
ttty_vformat:
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
	mv x27,x12
	mv x26,x13
	mv x25,x14
	mv x24,x15
	mv x23,x0 ; LOADU1
	j L.238
L.237:
	shlli x30,x22,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,37
	bne x30,x29,L.240
	mv x21,x0 ; LOADU1
	mv x20,x0 ; LOADI1
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	lbu x23,0(x30)
	shlli x30,x23,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.242
	j L.234
L.242:
	shlli x30,x23,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	bne x30,x29,L.244
	li x20,1
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	lbu x23,0(x30)
L.244:
	shlli x30,x23,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.246
	j L.234
L.246:
	shlli x30,x23,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	blt x30,x29,L.248
	li x29,57
	bgt x30,x29,L.248
	li x30,10
	shlli x29,x21,8*(4-1)
	shrli x29,x29,8*(4-1)
	imul x0,x30,x30,x29
	shlli x29,x23,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,-48
	add x30,x30,x29
	mv x21,x30 ; LOADU1
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	lbu x23,0(x30)
L.248:
	shlli x30,x23,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.250
	j L.234
L.250:
	shlli x19,x23,8*(4-1)
	shrli x19,x19,8*(4-1)
	li x30,99
	beq x19,x30,L.261
	li x30,100
	beq x19,x30,L.276
	bgt x19,x30,L.303
L.302:
	li x30,37
	beq x19,x30,L.301
	j L.241
L.303:
	li x30,115
	beq x19,x30,L.255
	blt x19,x30,L.241
L.304:
	li x30,120
	beq x19,x30,L.262
	j L.241
L.255:
	li x30,4
	add x24,x24,x30
	li x30,-4
	add x30,x24,x30
	lw x18,0(x30)
	mv x30,x18 ; LOADU4
	bne x30,x0,L.259
	la x30,L.235
	lw x18,0(x30)
	j L.259
L.258:
	mv x12,x26 ; LOADP4
	mv x30,x18 ; LOADP4
	li x29,1
	add x18,x30,x29
	lbu x13,0(x30)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
L.259:
	lbu x30,0(x18)
	bne x30,x0,L.258
	j L.241
L.261:
	li x30,4
	add x24,x24,x30
	li x30,-4
	add x30,x24,x30
	lw x30,0(x30)
	sb x30,-5+48(x8)
	mv x12,x26 ; LOADP4
	lbu x13,-5+48(x8)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
	j L.241
L.262:
	addi x30,x8,-42+48
	la x5,L.263
	li	x7,0
	add	x30,x30,x7
	li	x7,0
	add	x5,x5,x7
	li	x7,17
	copy	x5,x30,x7
	mv x18,x0 ; LOADI4
	li x30,4
	add x24,x24,x30
	li x30,-4
	add x30,x24,x30
	lw x30,0(x30)
	sw x30,-8+48(x8)
	j L.265
L.264:
	mv x30,x18 ; LOADI4
	addi x18,x30,1
	addi x29,x8,-25+48
	add x30,x30,x29
	lw x29,-8+48(x8)
	li x28,15
	and x29,x29,x28
	addi x28,x8,-42+48
	add x29,x29,x28
	lbu x29,0(x29)
	sb x29,0(x30)
	lw x30,-8+48(x8)
	li x29,4
	shrl x30,x30,x29
	sw x30,-8+48(x8)
L.265:
	lw x30,-8+48(x8)
	bne x30,x0,L.264
	mv x30,x18 ; LOADU4
	sb x30,-9+48(x8)
	j L.268
L.267:
	mv x12,x26 ; LOADP4
	shlli x30,x20,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.271
	li x30,48
	sw x30,-48+48(x8)
	j L.272
L.271:
	li x30,32
	sw x30,-48+48(x8)
L.272:
	lw x30,-48+48(x8)
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
	lbu x30,-9+48(x8)
	addi x30,x30,1
	sb x30,-9+48(x8)
L.268:
	lbu x30,-9+48(x8)
	shlli x29,x21,8*(4-1)
	shrli x29,x29,8*(4-1)
	blt x30,x29,L.267
	j L.274
L.273:
	mv x12,x26 ; LOADP4
	addi x30,x18,-1
	mv x18,x30 ; LOADI4
	addi x29,x8,-25+48
	add x30,x30,x29
	lbu x13,0(x30)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
L.274:
	bgt x18,x0,L.273
	j L.241
L.276:
	mv x18,x0 ; LOADI4
	li x30,4
	add x24,x24,x30
	li x30,-4
	add x30,x24,x30
	lw x30,0(x30)
	sw x30,-36+48(x8)
	sw x0,-12+48(x8)
	lw x30,-36+48(x8)
	bge x30,x0,L.278
	li x30,1
	sw x30,-40+48(x8)
	j L.279
L.278:
	sw x0,-40+48(x8)
L.279:
	lw x30,-40+48(x8)
	sb x30,-29+48(x8)
	lb x30,-29+48(x8)
	beq x30,x0,L.280
	lw x30,-36+48(x8)
	sub x30,x0,x30
	sw x30,-8+48(x8)
	j L.283
L.280:
	lw x30,-36+48(x8)
	sw x30,-8+48(x8)
	j L.283
L.282:
	mv x30,x18 ; LOADI4
	addi x18,x30,1
	addi x29,x8,-28+48
	add x30,x30,x29
	lw x29,-8+48(x8)
	li x28,10
	udiv x0,x29,x29,x28
	li x28,48
	add x29,x29,x28
	sb x29,0(x30)
	lw x30,-8+48(x8)
	li x29,10
	udiv x30,x0,x30,x29
	sw x30,-8+48(x8)
L.283:
	lw x30,-8+48(x8)
	bne x30,x0,L.282
	shlli x29,x20,8*(4-1)
	shrai x29,x29,8*(4-1)
	beq x29,x0,L.285
	lb x29,-29+48(x8)
	beq x29,x0,L.285
	mv x12,x26 ; LOADP4
	li x13,45
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
L.285:
	lb x30,-29+48(x8)
	beq x30,x0,L.288
	li x30,1
	sw x30,-44+48(x8)
	j L.289
L.288:
	sw x0,-44+48(x8)
L.289:
	lw x30,-44+48(x8)
	add x30,x18,x30
	sw x30,-12+48(x8)
	j L.291
L.290:
	mv x12,x26 ; LOADP4
	shlli x30,x20,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.294
	li x30,48
	sw x30,-48+48(x8)
	j L.295
L.294:
	li x30,32
	sw x30,-48+48(x8)
L.295:
	lw x30,-48+48(x8)
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
	lw x30,-12+48(x8)
	addi x30,x30,1
	sw x30,-12+48(x8)
L.291:
	lw x30,-12+48(x8)
	shlli x29,x21,8*(4-1)
	shrli x29,x29,8*(4-1)
	blt x30,x29,L.290
	shlli x29,x20,8*(4-1)
	shrai x29,x29,8*(4-1)
	bne x29,x0,L.299
	lb x29,-29+48(x8)
	beq x29,x0,L.299
	mv x12,x26 ; LOADP4
	li x13,45
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
	j L.299
L.298:
	mv x12,x26 ; LOADP4
	addi x30,x18,-1
	mv x18,x30 ; LOADI4
	addi x29,x8,-28+48
	add x30,x30,x29
	lbu x13,0(x30)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
L.299:
	bgt x18,x0,L.298
	j L.241
L.301:
	mv x12,x26 ; LOADP4
	li x13,37
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
	j L.241
L.240:
	mv x12,x26 ; LOADP4
	shlli x13,x22,8*(4-1)
	shrli x13,x13,8*(4-1)
	mv x31,x27 ; LOADP4
	jalr x1, 0(x31)
L.241:
L.238:
	mv x30,x25 ; LOADP4
	li x29,1
	add x25,x30,x29
	lbu x30,0(x30)
	mv x22,x30 ; LOADU1
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	bne x30,x0,L.237
L.234:
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
ttty_putc_wrapper:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	lw x12,0+16(x8)
	lbu x13,4+16(x8)
	call ttty_putc
L.305:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
ttty_mux_putc_wrapper:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	lw x12,0+16(x8)
	lbu x13,4+16(x8)
	call ttty_mux_putc
L.306:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl ttty_printf
	.align	4
ttty_printf:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	sw x15,28(x8)
	sw x16,32(x8)
	sw x17,36(x8)
	addi x30,x8,8+16
	sw x30,-8+16(x8)
	la x12,ttty_putc_wrapper
	lw x13,0+16(x8)
	lw x14,4+16(x8)
	lw x15,-8+16(x8)
	call ttty_vformat
L.307:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl ttty_mux_printf
	.align	4
ttty_mux_printf:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	sw x15,28(x8)
	sw x16,32(x8)
	sw x17,36(x8)
	addi x30,x8,8+16
	sw x30,-8+16(x8)
	la x12,ttty_mux_putc_wrapper
	lw x13,0+16(x8)
	lw x14,4+16(x8)
	lw x15,-8+16(x8)
	call ttty_vformat
L.309:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
ANSI_UP:
	.word	L.311
	.align	4
ANSI_DOWN:
	.word	L.312
	.align	4
ANSI_LEFT:
	.word	L.313
	.align	4
ANSI_RIGHT:
	.word	L.314
	.align	4
ANSI_HOME:
	.word	L.315
	.align	4
ANSI_END:
	.word	L.316
	.align	4
ANSI_DELETE:
	.word	L.317
	.align	4
ANSI_PGUP:
	.word	L.318
	.align	4
ANSI_PGDOWN:
	.word	L.319
	.align	4
ANSI_F1:
	.word	L.320
	.align	4
ANSI_F2:
	.word	L.321
	.align	4
ANSI_F3:
	.word	L.322
	.align	4
ANSI_F4:
	.word	L.323
	.globl tins_getc
	.align 4
	.text
	.align	4
tins_getc:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	j L.326
L.325:
	mv x25,x0 ; LOADI4
L.328:
L.329:
	addi x25,x25,1
	li x30,10000
	blt x25,x30,L.328
L.326:
	lw x12,0(x27)
	li x30,8
	add x30,x27,x30
	lw x31,0(x30)
	jalr x1, 0(x31)
	mv x26,x10 ; LOADI4
	li x29,-1
	beq x10,x29,L.325
	mv x30,x26 ; LOADU4
	shlli x10,x30,8*(4-1)
	shrli x10,x10,8*(4-1)
L.324:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl tins_read
	.align	4
tins_read:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,0+16(x8)
	lw x12,0(x30)
	li x29,8
	add x30,x30,x29
	lw x31,0(x30)
	jalr x1, 0(x31)
	sw x10,-8+16(x8)
	lw x30,-8+16(x8)
	li x29,-1
	beq x30,x29,L.333
	lw x30,4+16(x8)
	lw x29,-8+16(x8)
	sb x29,0(x30)
	li x10,1
	j L.332
L.333:
	mv x10,x0 ; LOADI4
L.332:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
kbd_event_push:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	li x30,192
	add x30,x12,x30
	lbu x30,0(x30)
	addi x30,x30,1
	li x29,32
	idiv x0,x30,x30,x29
	mv x27,x30 ; LOADU1
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,193
	add x29,x12,x29
	lbu x29,0(x29)
	bne x30,x29,L.336
	mv x10,x0 ; LOADI4
	j L.335
L.336:
	li x30,6
	li x29,192
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	add x30,x30,x12
	lhu x29,0(x13)
	sh x29,0(x30)
	li x30,2
	li x29,6
	li x28,192
	add x28,x12,x28
	lbu x28,0(x28)
	imul x0,x29,x29,x28
	add x29,x29,x12
	add x29,x29,x30
	add x30,x13,x30
	lbu x30,0(x30)
	sb x30,0(x29)
	li x30,3
	li x29,6
	li x28,192
	add x28,x12,x28
	lbu x28,0(x28)
	imul x0,x29,x29,x28
	add x29,x29,x12
	add x29,x29,x30
	add x30,x13,x30
	lbu x30,0(x30)
	sb x30,0(x29)
	li x30,4
	li x29,6
	li x28,192
	add x28,x12,x28
	lbu x28,0(x28)
	imul x0,x29,x29,x28
	add x29,x29,x12
	add x29,x29,x30
	add x30,x13,x30
	lb x30,0(x30)
	sb x30,0(x29)
	li x30,192
	add x30,x12,x30
	sb x27,0(x30)
	li x10,1
L.335:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align	4
kbd_event_peek:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,192
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,193
	add x29,x12,x29
	lbu x29,0(x29)
	bne x30,x29,L.339
	mv x10,x0 ; LOADP4
	j L.338
L.339:
	li x30,6
	li x29,193
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	add x10,x30,x12
L.338:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align	4
kbd_event_pop:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,192
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,193
	add x29,x12,x29
	lbu x29,0(x29)
	bne x30,x29,L.342
	mv x10,x0 ; LOADP4
	j L.341
L.342:
	li x30,193
	add x30,x12,x30
	li x29,6
	lbu x28,0(x30)
	imul x0,x29,x29,x28
	add x29,x29,x12
	sw x29,-8+16(x8)
	lbu x29,0(x30)
	addi x29,x29,1
	li x28,32
	idiv x0,x29,x29,x28
	sb x29,0(x30)
	lw x10,-8+16(x8)
L.341:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align 4
	.bss
	.align	2
L.345:
	.space	6
	.globl bios_kbd_handler
	.align 4
	.text
	.align	4
bios_kbd_handler:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x27,28(x2)
	sb x0,-5+16(x8)
	la x30,Devices+2
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lwd
	mv x27,x10 ; LOADU4
	li x12,57005
	mv x13,x27 ; LOADU4
	li x30,16
	shrl x30,x27,x30
	li x29,255
	and x14,x30,x29
	li x30,0xffff7fff
	and x15,x27,x30
	call _trace
	la x30,L.345+3
	li x29,24
	shrl x29,x27,x29
	sb x29,0(x30)
	la x30,L.345+2
	li x29,16
	shrl x29,x27,x29
	li x28,255
	and x29,x29,x28
	sb x29,0(x30)
	la x30,L.345
	li x29,0xffff7fff
	and x29,x27,x29
	sh x29,0(x30)
	la x30,L.345+4
	li x29,0x8000
	and x29,x27,x29
	li x28,15
	shrl x29,x29,x28
	sb x29,0(x30)
	la x12,KeyboardEvents
	la x13,L.345
	call kbd_event_push
	mv x30,x10 ; LOADI1
	sb x30,-5+16(x8)
L.344:
	lw x1,24(x2)
	lw x27,28(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.bss
	.align	4
L.351:
	.space	8
	.align 4
	.text
	.align	4
kbd_event_to_ansi:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	la x30,L.351
	sw x0,0(x30)
	la x30,L.351+4
	sb x0,0(x30)
	lhu x27,0(x12)
	li x30,261
	blt x27,x30,L.353
	li x30,269
	bgt x27,x30,L.369
	li x30,2
	shll x30,x27,x30
	la x29,L.370-1044
	add x30,x30,x29
	lw x30,0(x30)
	jalr x0,0(x30)
	.align 4
	.data
	.align	4
L.370:
	.word	L.362
	.word	L.359
	.word	L.358
	.word	L.357
	.word	L.356
	.word	L.363
	.word	L.364
	.word	L.360
	.word	L.361
	.align 4
	.text
L.369:
	li x30,290
	blt x27,x30,L.353
	li x30,293
	bgt x27,x30,L.353
	li x30,2
	shll x30,x27,x30
	la x29,L.372-1160
	add x30,x30,x29
	lw x30,0(x30)
	jalr x0,0(x30)
	.align 4
	.data
	.align	4
L.372:
	.word	L.365
	.word	L.366
	.word	L.367
	.word	L.368
	.align 4
	.text
L.356:
	la x30,L.351
	la x29,ANSI_UP
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.357:
	la x30,L.351
	la x29,ANSI_DOWN
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.358:
	la x30,L.351
	la x29,ANSI_LEFT
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.359:
	la x30,L.351
	la x29,ANSI_RIGHT
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.360:
	la x30,L.351
	la x29,ANSI_HOME
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.361:
	la x30,L.351
	la x29,ANSI_END
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.362:
	la x30,L.351
	la x29,ANSI_DELETE
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.363:
	la x30,L.351
	la x29,ANSI_PGUP
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.364:
	la x30,L.351
	la x29,ANSI_PGDOWN
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.365:
	la x30,L.351
	la x29,ANSI_F1
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.366:
	la x30,L.351
	la x29,ANSI_F2
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.367:
	la x30,L.351
	la x29,ANSI_F3
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.368:
	la x30,L.351
	la x29,ANSI_F4
	lw x29,0(x29)
	sw x29,0(x30)
	mv x10,x30 ; LOADP4
	j L.350
L.353:
	la x30,L.351+4
	li x29,2
	add x29,x12,x29
	lbu x29,0(x29)
	sb x29,0(x30)
	la x10,L.351
L.350:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align 4
	.bss
	.align	4
L.376:
	.space	4
	.align 4
	.data
	.align	1
L.377:
	.byte	0x0
	.globl kbd_get_translated
	.align 4
	.text
	.align	4
kbd_get_translated:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	la x30,L.377
	lb x30,0(x30)
	bne x30,x0,L.378
	lw x12,0+16(x8)
	call kbd_event_peek
	sw x10,-12+16(x8)
	lw x30,4+16(x8)
	li x29,1
	sb x29,0(x30)
	lw x30,-12+16(x8)
	bne x30,x0,L.380
	li x10,-1
	j L.375
L.380:
	lw x12,-12+16(x8)
	call kbd_event_to_ansi
	la x29,L.376
	sw x10,0(x29)
	la x30,L.376
	lw x30,0(x30)
	lw x30,0(x30)
	bne x30,x0,L.382
	la x30,L.376
	lw x30,0(x30)
	li x29,4
	add x30,x30,x29
	lbu x30,0(x30)
	beq x30,x0,L.384
	la x30,L.376
	lw x30,0(x30)
	li x29,4
	add x30,x30,x29
	lbu x10,0(x30)
	j L.375
L.384:
	li x10,-1
	j L.375
L.382:
	la x30,L.377
	li x29,1
	sb x29,0(x30)
	la x30,L.376
	lw x30,0(x30)
	lw x29,0(x30)
	li x28,1
	add x28,x29,x28
	sw x28,0(x30)
	lbu x30,0(x29)
	sw x30,-8+16(x8)
	lw x10,-8+16(x8)
	j L.375
L.378:
	lw x30,4+16(x8)
	sb x0,0(x30)
	la x30,L.376
	lw x30,0(x30)
	lw x29,0(x30)
	li x28,1
	add x28,x29,x28
	sw x28,0(x30)
	lbu x30,0(x29)
	sw x30,-8+16(x8)
	beq x30,x0,L.386
	lw x10,-8+16(x8)
	j L.375
L.386:
	la x30,L.377
	sb x0,0(x30)
	li x10,-1
L.375:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
kbd_get_byte:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sb x0,-5+16(x8)
	lw x30,0+16(x8)
	sw x30,-12+16(x8)
	lw x12,-12+16(x8)
	addi x13,x8,-5+16
	call kbd_get_translated
	mv x27,x10 ; LOADI4
	lb x30,-5+16(x8)
	beq x30,x0,L.389
	lw x12,-12+16(x8)
	call kbd_event_pop
L.389:
	li x30,13
	bne x27,x30,L.392
	li x26,10
	j L.393
L.392:
	mv x26,x27 ; LOADI4
L.393:
	mv x10,x26 ; LOADI4
L.388:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl kbd_init_tins
	.align	4
kbd_init_tins:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	la x30,KeyboardEvents
	sw x30,0(x12)
	li x30,8
	add x30,x12,x30
	la x29,kbd_get_byte
	sw x29,0(x30)
L.394:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl serial_is_connected
	.align	4
serial_is_connected:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,Devices
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	sb x30,-5+16(x8)
	lbu x30,-5+16(x8)
	andi x30,x30,2
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.395:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl serial_available
	.align	4
serial_available:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,Devices
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	sb x30,-5+16(x8)
	lbu x30,-5+16(x8)
	andi x30,x30,1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.396:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl serial_outc
	.align	4
serial_outc:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	lw x30,0+16(x8)
	li x29,16
	add x30,x30,x29
	lw x30,0(x30)
	lbu x30,0(x30)
	sb x30,-5+16(x8)
	lbu x30,-5+16(x8)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	sb x30,-6+16(x8)
	lbu x30,-6+16(x8)
	andi x30,x30,2
	beq x30,x0,L.398
	lbu x30,-5+16(x8)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,4+16(x8)
	call _sbd
L.398:
L.397:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl video_text_screen_size
	.align	4
video_text_screen_size:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,3
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,4
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	lw x29,8+16(x8)
	mv x30,x10 ; LOADU4
	sb x30,0(x29)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	lw x29,0+16(x8)
	mv x30,x10 ; LOADU4
	sb x30,0(x29)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,6
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	lw x29,4+16(x8)
	mv x30,x10 ; LOADU4
	sb x30,0(x29)
L.400:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl video_set_cursor
	.align	4
video_set_cursor:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,12
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0+16(x8)
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,13
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,4+16(x8)
	call _sbd
L.405:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl video_set_csr
	.align	4
video_set_csr:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0+16(x8)
	call _sbd
L.408:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl video_get_cursor
	.align	4
video_get_cursor:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,12
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	lw x29,0+16(x8)
	mv x30,x10 ; LOADU4
	sb x30,0(x29)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,13
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	lw x29,4+16(x8)
	mv x30,x10 ; LOADU4
	sb x30,0(x29)
L.410:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl video_get_csr
	.align	4
video_get_csr:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	shlli x10,x30,8*(4-1)
	shrli x10,x10,8*(4-1)
L.413:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
video_text_mode_default_attr:
	.byte	0xf
	.byte	0x0
	.byte	0x4
	.globl video_outc
	.align 4
	.text
	.align	4
video_outc:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x25,0(x2)
	sw x26,4(x2)
	sw x27,8(x2)
	li x30,16
	add x30,x12,x30
	lw x27,0(x30)
	li x30,7
	add x30,x12,x30
	lbu x30,0(x30)
	shlli x30,x30,8*(4-2)
	shrli x30,x30,8*(4-2)
	li x29,4
	add x29,x12,x29
	lbu x29,0(x29)
	shlli x29,x29,8*(4-2)
	shrli x29,x29,8*(4-2)
	imul x0,x30,x30,x29
	li x29,6
	add x29,x12,x29
	lbu x29,0(x29)
	shlli x29,x29,8*(4-2)
	shrli x29,x29,8*(4-2)
	add x30,x30,x29
	li x29,8
	add x29,x12,x29
	lbu x29,0(x29)
	imul x0,x30,x30,x29
	mv x26,x30 ; LOADU2
	shlli x30,x26,8*(4-2)
	shrli x30,x30,8*(4-2)
	add x30,x30,x27
	sb x13,0(x30)
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	li x29,1
	ble x30,x29,L.416
	li x25,1
	j L.421
L.418:
	shlli x30,x26,8*(4-2)
	shrli x30,x30,8*(4-2)
	add x30,x30,x25
	add x30,x30,x27
	li x29,1
	sub x29,x25,x29
	li x28,56
	add x28,x12,x28
	add x29,x29,x28
	lbu x29,0(x29)
	sb x29,0(x30)
L.419:
	li x30,1
	add x25,x25,x30
L.421:
	li x30,8
	add x30,x12,x30
	lbu x30,0(x30)
	bltu x25,x30,L.418
L.416:
L.415:
	lw x25,0(x2)
	lw x26,4(x2)
	lw x27,8(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl video_setpos
	.align	4
video_setpos:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	lw x30,8+16(x8)
	sb x30,8+16(x8)
	lbu x12,4+16(x8)
	lbu x13,8+16(x8)
	call video_set_cursor
L.422:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
find_devices:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x23,28(x2)
	sw x24,32(x2)
	sw x25,36(x2)
	sw x26,40(x2)
	sw x27,44(x2)
	mv x27,x0 ; LOADU1
	j L.427
L.424:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,256
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	mv x26,x30 ; LOADU1
	shlli x23,x26,8*(4-1)
	shrli x23,x23,8*(4-1)
	li x30,75
	beq x23,x30,L.431
	li x30,77
	beq x23,x30,L.435
	bgt x23,x30,L.437
L.436:
	li x30,65
	beq x23,x30,L.434
	li x30,68
	beq x23,x30,L.433
	j L.429
L.437:
	li x30,86
	beq x23,x30,L.432
	j L.429
L.431:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,4
	shll x30,x30,x29
	mv x25,x30 ; LOADU1
	j L.429
L.432:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,4
	shll x30,x30,x29
	sb x30,-6+16(x8)
	j L.429
L.433:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,4
	shll x30,x30,x29
	mv x24,x30 ; LOADU1
	j L.429
L.434:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,4
	shll x30,x30,x29
	sb x30,-5+16(x8)
	j L.429
L.435:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,4
	shll x30,x30,x29
	sb x30,-7+16(x8)
L.429:
L.425:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	mv x27,x30 ; LOADU1
L.427:
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,16
	blt x30,x29,L.424
	la x30,Devices
	sb x25,0(x30)
	la x30,Devices+1
	shlli x29,x25,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,6
	sb x29,0(x30)
	la x30,Devices+2
	shlli x29,x25,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,12
	sb x29,0(x30)
	la x30,Devices+3
	lbu x29,-6+16(x8)
	sb x29,0(x30)
	la x30,Devices+4
	sb x24,0(x30)
	la x30,Devices+5
	shlli x29,x24,8*(4-1)
	shrli x29,x29,8*(4-1)
	addi x29,x29,6
	sb x29,0(x30)
	la x30,Devices+6
	lbu x29,-5+16(x8)
	sb x29,0(x30)
	la x30,Devices+7
	lbu x29,-7+16(x8)
	sb x29,0(x30)
L.423:
	lw x1,24(x2)
	lw x23,28(x2)
	lw x24,32(x2)
	lw x25,36(x2)
	lw x26,40(x2)
	lw x27,44(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.align	4
bios_init:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,40(x2)
	sw x26,44(x2)
	sw x27,48(x2)
	la x30,KeyboardEvents+192
	sb x0,0(x30)
	la x30,KeyboardEvents+193
	sb x0,0(x30)
	li x12,240
	call _lbud
	la x29,Memsize
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x28,20
	shll x30,x30,x28
	sw x30,0(x29)
	la x30,Stack_top
	la x29,Memsize
	lw x29,0(x29)
	li x28,0x8000
	sub x29,x29,x28
	sw x29,0(x30)
	la x29,Stack_base
	lw x30,0(x30)
	li x28,0x10000
	sub x30,x30,x28
	sw x30,0(x29)
	call find_devices
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,2
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,5
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,6
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
	mv x27,x0 ; LOADU4
	j L.456
L.453:
	add x26,x26,x27
L.454:
	li x30,1
	add x27,x27,x30
L.456:
	li x30,0x186a0
	bltu x27,x30,L.453
	addi x12,x8,-6+16
	addi x13,x8,-5+16
	addi x14,x8,-7+16
	call video_text_screen_size
	la x30,Charbuff_size
	lbu x29,-5+16(x8)
	lbu x28,-6+16(x8)
	lbu x7,-7+16(x8)
	sw x7,-12+16(x8)
	imul x0,x6,x29,x28
	imul x0,x6,x6,x7
	sw x6,0(x30)
	la x6,Charbuff
	la x5,Stack_base
	lw x5,0(x5)
	li x7,1024
	sub x7,x5,x7
	lw x30,0(x30)
	sub x30,x7,x30
	sw x30,0(x6)
	li x12,1
	mv x13,x28 ; LOADI4
	mv x14,x29 ; LOADI4
	lw x30,-12+16(x8)
	mv x15,x30 ; LOADI4
	call _trace
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,2
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,0xffffffff
	call _swd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	la x30,Charbuff
	lw x30,0(x30)
	mv x13,x30 ; LOADU4
	call _swd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,7
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,0x20000000
	call _swd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,5
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
	mv x27,x0 ; LOADU4
	j L.469
L.466:
	add x26,x26,x27
L.467:
	li x30,1
	add x27,x27,x30
L.469:
	li x30,0xf429a
	bltu x27,x30,L.466
	la x12,Video_tty
	la x13,L.470
	li x14,129
	lbu x15,-6+16(x8)
	lbu x16,-5+16(x8)
	lbu x17,-7+16(x8)
	la x30,Charbuff
	lw x30,0(x30)
	sw x30,24(x2)
	la x30,video_text_mode_default_attr
	sw x30,28(x2)
	la x30,video_outc
	sw x30,32(x2)
	la x30,video_setpos
	sw x30,36(x2)
	call ttty_init
	la x12,Serial_tty
	la x13,L.471
	mv x14,x0 ; LOADI4
	li x30,255
	mv x15,x30 ; LOADI4
	mv x16,x30 ; LOADI4
	li x17,1
	la x30,Devices
	sw x30,24(x2)
	sw x0,28(x2)
	la x30,serial_outc
	sw x30,32(x2)
	sw x0,36(x2)
	call ttty_init
	la x12,Con
	la x13,Video_tty
	li x14,10
	call ttty_mux_subscribe
	la x12,Con
	la x13,Serial_tty
	li x14,10
	call ttty_mux_subscribe
	la x12,Con
	li x13,255
	call ttty_mux_set_priority
	la x30,Devices+2
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,129
	call _sbd
	la x12,Keyboard_in
	call kbd_init_tins
	la x30,CurrentTps
	sw x0,0(x30)
L.445:
	lw x1,40(x2)
	lw x26,44(x2)
	lw x27,48(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl ushell_putchar
	.align	4
ushell_putchar:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	la x12,Con
	lbu x13,0+16(x8)
	call ttty_mux_putc
L.473:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl sysinfo
	.align	4
sysinfo:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,0+16(x8)
	li x29,1
	beq x30,x29,L.475
	la x12,Con
	la x13,L.477
	call ttty_mux_puts
	j L.474
L.475:
	la x12,Con
	la x13,L.478
	call ttty_mux_puts
	la x12,Con
	la x13,L.479
	call ttty_mux_puts
	la x12,Con
	la x13,L.480
	call ttty_mux_puts
	li x12,241
	call _lbud
	li x29,353
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	imul x0,x30,x29,x30
	addi x30,x30,10000
	sw x30,-12+16(x8)
	li x12,253
	call _lbud
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,83
	bne x30,x29,L.488
	la x27,L.482
	j L.489
L.488:
	la x27,L.483
L.489:
	li x12,254
	call _lbud
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,84
	bne x30,x29,L.490
	la x26,L.485
	j L.491
L.490:
	la x26,L.486
L.491:
	la x12,Con
	la x13,L.481
	mv x14,x27 ; LOADP4
	mv x15,x26 ; LOADP4
	lw x16,-12+16(x8)
	call ttty_mux_printf
	li x12,240
	call _lbud
	la x12,Con
	la x13,L.492
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,10
	shll x14,x30,x29
	call ttty_mux_printf
	la x12,Con
	la x13,L.493
	call ttty_mux_puts
	li x12,248
	li x13,1
	call _sbd
	li x12,248
	call _lwd
	sw x10,-8+16(x8)
	la x12,Con
	la x13,L.494
	lw x30,-8+16(x8)
	li x29,1000
	udiv x14,x0,x30,x29
	call ttty_mux_printf
	la x12,Con
	la x13,L.495
	call ttty_mux_puts
	call serial_is_connected
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.500
	la x25,L.497
	j L.501
L.500:
	la x25,L.498
L.501:
	la x12,Con
	la x13,L.496
	la x30,Devices
	lbu x14,0(x30)
	li x15,75
	mv x16,x25 ; LOADP4
	call ttty_mux_printf
	la x12,Con
	la x13,L.502
	la x30,Devices+1
	lbu x14,0(x30)
	li x15,75
	call ttty_mux_printf
	la x12,Con
	la x13,L.504
	la x30,Devices+2
	lbu x14,0(x30)
	li x15,75
	call ttty_mux_printf
	la x12,Con
	la x13,L.506
	la x30,Devices+3
	lbu x14,0(x30)
	li x15,86
	call ttty_mux_printf
	la x12,Con
	la x13,L.508
	la x30,Devices+4
	lbu x14,0(x30)
	li x15,68
	call ttty_mux_printf
	la x12,Con
	la x13,L.510
	la x30,Devices+5
	lbu x14,0(x30)
	li x15,68
	call ttty_mux_printf
	la x12,Con
	la x13,L.512
	la x30,Devices+6
	lbu x14,0(x30)
	li x15,65
	call ttty_mux_printf
	la x12,Con
	la x13,L.514
	la x30,Devices+7
	lbu x14,0(x30)
	li x15,77
	call ttty_mux_printf
L.474:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl echo
	.align	4
echo:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	mv x27,x12
	mv x26,x13
	li x30,2
	blt x27,x30,L.517
	li x30,4
	add x30,x26,x30
	lw x30,0(x30)
	lbu x30,0(x30)
	li x29,45
	bne x30,x29,L.517
	li x30,4
	add x30,x26,x30
	lw x30,0(x30)
	li x29,1
	add x30,x30,x29
	lbu x30,0(x30)
	sb x30,-5+16(x8)
	lbu x25,-5+16(x8)
	li x30,97
	beq x25,x30,L.528
	blt x25,x30,L.519
L.539:
	li x30,101
	beq x25,x30,L.534
	li x30,104
	beq x25,x30,L.523
	j L.519
L.522:
L.523:
	la x12,Con
	la x13,L.524
	call ttty_mux_printf
	la x12,Con
	la x13,L.525
	call ttty_mux_printf
	la x12,Con
	la x13,L.526
	call ttty_mux_printf
	la x12,Con
	la x13,L.527
	call ttty_mux_printf
	j L.516
L.528:
	sw x0,-12+16(x8)
	li x30,2
	sw x30,-12+16(x8)
	j L.532
L.529:
	la x12,Con
	la x13,L.533
	lw x30,-12+16(x8)
	li x29,2
	shll x30,x30,x29
	add x30,x30,x26
	lw x14,0(x30)
	call ttty_mux_printf
	la x12,Con
	li x13,32
	call ttty_mux_putc
L.530:
	lw x30,-12+16(x8)
	addi x30,x30,1
	sw x30,-12+16(x8)
L.532:
	lw x30,-12+16(x8)
	blt x30,x27,L.529
	la x12,Con
	li x13,10
	call ttty_mux_putc
	j L.516
L.534:
L.519:
	sw x0,-12+16(x8)
	li x30,2
	sw x30,-12+16(x8)
	j L.538
L.535:
	la x12,Con
	lw x30,-12+16(x8)
	li x29,2
	shll x30,x30,x29
	add x30,x30,x26
	lw x13,0(x30)
	call ttty_mux_puts
	la x12,Con
	li x13,32
	call ttty_mux_putc
L.536:
	lw x30,-12+16(x8)
	addi x30,x30,1
	sw x30,-12+16(x8)
L.538:
	lw x30,-12+16(x8)
	blt x30,x27,L.535
	la x12,Con
	li x13,10
	call ttty_mux_putc
	j L.516
L.517:
	li x30,1
	ble x27,x30,L.522
	mv x25,x0 ; LOADI4
	li x25,1
	j L.545
L.542:
	la x12,Con
	li x30,2
	shll x30,x25,x30
	add x30,x30,x26
	lw x13,0(x30)
	call ttty_mux_puts
	la x12,Con
	li x13,32
	call ttty_mux_putc
L.543:
	addi x25,x25,1
L.545:
	blt x25,x27,L.542
	la x12,Con
	li x13,10
	call ttty_mux_putc
L.516:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl strchr
	.align	4
strchr:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
L.547:
	lbu x30,0(x12)
	bne x30,x13,L.550
	mv x10,x12 ; LOADP4
	j L.546
L.550:
L.548:
	mv x30,x12 ; LOADP4
	li x29,1
	add x12,x30,x29
	lbu x30,0(x30)
	bne x30,x0,L.547
	mv x10,x0 ; LOADP4
L.546:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl to_lower
	.align	4
to_lower:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,65
	blt x30,x29,L.553
	li x29,90
	bgt x30,x29,L.553
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,32
	shlli x10,x30,8*(4-1)
	shrli x10,x10,8*(4-1)
	j L.552
L.553:
	mv x10,x0 ; LOADI4
L.552:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl is_space
	.align	4
is_space:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,32
	beq x30,x29,L.560
	li x29,9
	beq x30,x29,L.560
	li x29,10
	bne x30,x29,L.557
L.560:
	li x27,1
	j L.558
L.557:
	mv x27,x0 ; LOADI4
L.558:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.555:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl strtol
	.align	4
strtol:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x21,28(x2)
	sw x22,32(x2)
	sw x23,36(x2)
	sw x24,40(x2)
	sw x25,44(x2)
	sw x26,48(x2)
	sw x27,52(x2)
	mv x27,x12
	sw x13,20(x8)
	mv x26,x14
	li x30,1
	sw x30,-8+16(x8)
	mv x22,x0 ; LOADI4
	mv x23,x0 ; LOADI4
	blt x26,x0,L.564
	li x30,36
	ble x26,x30,L.562
L.564:
	mv x10,x0 ; LOADI4
	j L.561
L.562:
	sw x27,-12+16(x8)
	j L.566
L.565:
	li x30,1
	add x27,x27,x30
L.566:
	lbu x12,0(x27)
	call is_space
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	bne x30,x0,L.565
	lbu x30,0(x27)
	li x29,45
	bne x30,x29,L.568
	li x30,-1
	sw x30,-8+16(x8)
	li x30,1
	add x27,x27,x30
	j L.569
L.568:
	lbu x30,0(x27)
	li x29,43
	bne x30,x29,L.570
	li x30,1
	add x27,x27,x30
L.570:
L.569:
	beq x26,x0,L.574
	li x30,16
	bne x26,x30,L.572
L.574:
	lbu x30,0(x27)
	li x29,48
	bne x30,x29,L.572
	li x30,1
	add x30,x27,x30
	lbu x30,0(x30)
	li x29,120
	bne x30,x29,L.572
	li x26,16
	li x30,2
	add x27,x27,x30
	j L.573
L.572:
	beq x26,x0,L.577
	li x30,8
	bne x26,x30,L.575
L.577:
	lbu x30,0(x27)
	li x29,48
	bne x30,x29,L.575
	li x26,8
	li x30,1
	add x27,x27,x30
L.575:
L.573:
	bne x26,x0,L.578
	li x26,10
L.578:
	la x24,L.580
L.581:
	lbu x12,0(x27)
	call to_lower
	mv x12,x24 ; LOADP4
	mv x30,x10 ; LOADU4
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	call strchr
	mv x25,x10 ; LOADP4
	beq x25,x0,L.587
	mv x29,x24 ; LOADU4
	sub x30,x25,x29
	blt x30,x26,L.585
L.587:
	j L.583
L.585:
	imul x0,x30,x23,x26
	add x30,x30,x25
	mv x29,x24 ; LOADU4
	sub x30,x30,x29
	mv x23,x30 ; LOADI4
	li x30,1
	add x27,x27,x30
	li x22,1
	j L.581
L.583:
	lw x30,4+16(x8)
	beq x30,x0,L.588
	beq x22,x0,L.591
	mv x21,x27 ; LOADP4
	j L.592
L.591:
	lw x21,-12+16(x8)
L.592:
	lw x30,4+16(x8)
	sw x21,0(x30)
L.588:
	lw x30,-8+16(x8)
	imul x0,x10,x23,x30
L.561:
	lw x1,24(x2)
	lw x21,28(x2)
	lw x22,32(x2)
	lw x23,36(x2)
	lw x24,40(x2)
	lw x25,44(x2)
	lw x26,48(x2)
	lw x27,52(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl is_print
	.align	4
is_print:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,32
	blt x30,x29,L.595
	li x29,127
	beq x30,x29,L.595
	li x27,1
	j L.596
L.595:
	mv x27,x0 ; LOADI4
L.596:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.593:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl is_digit
	.align	4
is_digit:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	blt x30,x29,L.599
	li x29,57
	bgt x30,x29,L.599
	li x27,1
	j L.600
L.599:
	mv x27,x0 ; LOADI4
L.600:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.597:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl is_alpha
	.align	4
is_alpha:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,65
	blt x30,x29,L.603
	li x29,122
	bgt x30,x29,L.603
	li x27,1
	j L.604
L.603:
	mv x27,x0 ; LOADI4
L.604:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.601:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl is_upper
	.align	4
is_upper:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,65
	blt x30,x29,L.607
	li x29,90
	bgt x30,x29,L.607
	li x27,1
	j L.608
L.607:
	mv x27,x0 ; LOADI4
L.608:
	mv x30,x27 ; LOADI1
	shlli x10,x30,8*(4-1)
	shrai x10,x10,8*(4-1)
L.605:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl strtoul
	.align	4
strtoul:
	addi x2,x2,-96
	sw  x8,92(x2)
	addi  x8,x2,80
	sw x1,24(x2)
	sw x19,28(x2)
	sw x20,32(x2)
	sw x21,36(x2)
	sw x22,40(x2)
	sw x23,44(x2)
	sw x24,48(x2)
	sw x25,52(x2)
	sw x26,56(x2)
	sw x27,60(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x27,x14
	lw x24,0+16(x8)
	sw x0,-8+16(x8)
L.610:
	mv x30,x24 ; LOADP4
	li x29,1
	add x24,x30,x29
	lbu x26,0(x30)
L.611:
	shlli x12,x26,8*(4-1)
	shrli x12,x12,8*(4-1)
	call is_space
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	bne x30,x0,L.610
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,45
	bne x30,x29,L.613
	li x30,1
	sw x30,-8+16(x8)
	mv x29,x24 ; LOADP4
	add x24,x29,x30
	lbu x26,0(x29)
	j L.614
L.613:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,43
	bne x30,x29,L.615
	mv x30,x24 ; LOADP4
	li x29,1
	add x24,x30,x29
	lbu x26,0(x30)
L.615:
L.614:
	beq x27,x0,L.619
	li x30,16
	bne x27,x30,L.617
L.619:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	bne x30,x29,L.617
	lbu x30,0(x24)
	li x29,120
	beq x30,x29,L.620
	li x29,88
	bne x30,x29,L.617
L.620:
	li x30,1
	add x30,x24,x30
	lbu x26,0(x30)
	li x30,2
	add x24,x24,x30
	li x27,16
L.617:
	bne x27,x0,L.621
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,48
	bne x30,x29,L.624
	li x20,8
	j L.625
L.624:
	li x20,10
L.625:
	mv x27,x20 ; LOADI4
L.621:
	li x30,0xffffff
	udiv x22,x0,x30,x27
	udiv x0,x30,x30,x27
	mv x21,x30 ; LOADI4
	mv x25,x0 ; LOADU4
	mv x23,x0 ; LOADI4
L.626:
	shlli x12,x26,8*(4-1)
	shrli x12,x12,8*(4-1)
	call is_print
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	bne x30,x0,L.630
	j L.628
L.630:
	shlli x12,x26,8*(4-1)
	shrli x12,x12,8*(4-1)
	call is_digit
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.632
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,-48
	mv x26,x30 ; LOADU1
	j L.633
L.632:
	shlli x12,x26,8*(4-1)
	shrli x12,x12,8*(4-1)
	call is_alpha
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.628
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	sw x30,-12+16(x8)
	mv x12,x30 ; LOADI4
	call is_upper
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.637
	li x19,55
	j L.638
L.637:
	li x19,87
L.638:
	lw x30,-12+16(x8)
	sub x30,x30,x19
	mv x26,x30 ; LOADU1
L.635:
L.633:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	blt x30,x27,L.639
	j L.628
L.639:
	blt x23,x0,L.644
	bgtu x25,x22,L.644
	bne x25,x22,L.641
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	ble x30,x21,L.641
L.644:
	li x23,-1
	j L.642
L.641:
	li x23,1
	mv x30,x27 ; LOADU4
	imul x0,x25,x25,x30
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	add x25,x25,x30
L.642:
L.627:
	mv x30,x24 ; LOADP4
	li x29,1
	add x24,x30,x29
	lbu x26,0(x30)
	j L.626
L.628:
	bge x23,x0,L.645
	li x25,0xffffff
	j L.646
L.645:
	lw x30,-8+16(x8)
	beq x30,x0,L.647
	not x30,x25
	li x29,1
	add x25,x30,x29
L.647:
L.646:
	lw x30,4+16(x8)
	beq x30,x0,L.649
	beq x23,x0,L.652
	li x30,-1
	add x19,x24,x30
	j L.653
L.652:
	lw x19,0+16(x8)
L.653:
	lw x30,4+16(x8)
	sw x19,0(x30)
L.649:
	mv x10,x25 ; LOADU4
L.609:
	lw x1,24(x2)
	lw x19,28(x2)
	lw x20,32(x2)
	lw x21,36(x2)
	lw x22,40(x2)
	lw x23,44(x2)
	lw x24,48(x2)
	lw x25,52(x2)
	lw x26,56(x2)
	lw x27,60(x2)
	lw  x8,92(x2)
	addi  x2,x2,96
	jalr x0,0(x1)

	.globl peek
	.align	4
peek:
	addi x2,x2,-96
	sw  x8,92(x2)
	addi  x8,x2,80
	sw x1,40(x2)
	sw x21,44(x2)
	sw x22,48(x2)
	sw x23,52(x2)
	sw x24,56(x2)
	sw x25,60(x2)
	sw x26,64(x2)
	sw x27,68(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x26,x0 ; LOADU4
	mv x27,x0 ; LOADU4
	lw x30,0+16(x8)
	li x29,2
	beq x30,x29,L.655
	la x12,Con
	la x13,L.657
	call ttty_mux_printf
	la x12,Con
	la x13,L.658
	call ttty_mux_printf
	j L.654
L.655:
	lw x30,4+16(x8)
	li x29,4
	add x30,x30,x29
	lw x12,0(x30)
	mv x13,x0 ; LOADP4
	mv x14,x0 ; LOADI4
	call strtoul
	mv x26,x10 ; LOADU4
	li x30,128
	add x25,x26,x30
	li x30,-16
	and x27,x26,x30
	j L.662
L.659:
	mv x24,x0 ; LOADU1
	mv x23,x0 ; LOADP4
	la x12,Con
	la x13,L.663
	mv x14,x26 ; LOADU4
	mv x30,x26 ; LOADP4
	lbu x15,0(x30)
	call ttty_mux_printf
	la x12,Con
	la x13,L.664
	mv x14,x27 ; LOADU4
	call ttty_mux_printf
	la x12,Con
	la x13,L.665
	add x30,x27,x23
	lbu x14,0(x30)
	li x30,1
	add x30,x27,x30
	add x30,x30,x23
	lbu x15,0(x30)
	li x30,2
	add x30,x27,x30
	add x30,x30,x23
	lbu x16,0(x30)
	li x30,3
	add x30,x27,x30
	add x30,x30,x23
	lbu x17,0(x30)
	li x30,4
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,24(x2)
	li x30,5
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,28(x2)
	li x30,6
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,32(x2)
	li x30,7
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,36(x2)
	call ttty_mux_printf
	la x12,Con
	la x13,L.666
	li x30,8
	add x30,x27,x30
	add x30,x30,x23
	lbu x14,0(x30)
	li x30,9
	add x30,x27,x30
	add x30,x30,x23
	lbu x15,0(x30)
	li x30,10
	add x30,x27,x30
	add x30,x30,x23
	lbu x16,0(x30)
	li x30,11
	add x30,x27,x30
	add x30,x30,x23
	lbu x17,0(x30)
	li x30,12
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,24(x2)
	li x30,13
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,28(x2)
	li x30,14
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,32(x2)
	li x30,15
	add x30,x27,x30
	add x30,x30,x23
	lbu x30,0(x30)
	sw x30,36(x2)
	call ttty_mux_printf
	la x12,Con
	li x13,124
	call ttty_mux_putc
	mv x24,x0 ; LOADU1
	j L.670
L.667:
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	add x30,x30,x27
	add x30,x30,x23
	lbu x30,0(x30)
	beq x30,x0,L.672
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	add x30,x30,x27
	add x30,x30,x23
	lbu x21,0(x30)
	j L.673
L.672:
	li x21,46
L.673:
	mv x30,x21 ; LOADU4
	mv x22,x30 ; LOADU1
	la x12,Con
	shlli x13,x22,8*(4-1)
	shrli x13,x13,8*(4-1)
	call ttty_mux_putc
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,7
	bne x30,x29,L.674
	la x12,Con
	li x13,124
	call ttty_mux_putc
L.674:
L.668:
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	mv x24,x30 ; LOADU1
L.670:
	shlli x30,x24,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,16
	blt x30,x29,L.667
	la x12,Con
	li x13,124
	call ttty_mux_putc
	la x12,Con
	li x13,10
	call ttty_mux_putc
L.660:
	li x30,16
	add x27,x27,x30
L.662:
	bltu x27,x25,L.659
L.654:
	lw x1,40(x2)
	lw x21,44(x2)
	lw x22,48(x2)
	lw x23,52(x2)
	lw x24,56(x2)
	lw x25,60(x2)
	lw x26,64(x2)
	lw x27,68(x2)
	lw  x8,92(x2)
	addi  x2,x2,96
	jalr x0,0(x1)

	.globl poke
	.align	4
poke:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x27,x0 ; LOADU4
	mv x26,x0 ; LOADU4
	lw x30,0+16(x8)
	li x29,3
	beq x30,x29,L.677
	la x12,Con
	la x13,L.679
	call ttty_mux_printf
	la x12,Con
	la x13,L.680
	call ttty_mux_printf
	la x12,Con
	la x13,L.681
	call ttty_mux_printf
	j L.676
L.677:
	lw x30,4+16(x8)
	li x29,4
	add x30,x30,x29
	lw x12,0(x30)
	mv x13,x0 ; LOADP4
	mv x14,x0 ; LOADI4
	call strtoul
	mv x27,x10 ; LOADU4
	lw x30,4+16(x8)
	li x29,8
	add x30,x30,x29
	lw x12,0(x30)
	mv x13,x0 ; LOADP4
	mv x14,x0 ; LOADI4
	call strtoul
	mv x26,x10 ; LOADU4
	mv x30,x27 ; LOADP4
	li x29,255
	and x29,x26,x29
	sb x29,0(x30)
L.676:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl clear
	.align	4
clear:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	la x12,Con
	la x13,L.683
	call ttty_mux_puts
L.682:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl send_escape
	.align	4
send_escape:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x27,x0 ; LOADI4
	mv x26,x0 ; LOADU4
	mv x27,x0 ; LOADI4
L.685:
	mv x30,x27 ; LOADU4
	add x26,x26,x30
L.686:
	addi x27,x27,1
	li x30,1000000
	blt x27,x30,L.685
	la x30,Devices
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,43
	call _sbd
	mv x27,x0 ; LOADI4
L.689:
	mv x30,x27 ; LOADU4
	add x26,x26,x30
L.690:
	addi x27,x27,1
	li x30,10000
	blt x27,x30,L.689
	la x30,Devices
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,43
	call _sbd
	mv x27,x0 ; LOADI4
L.693:
	mv x30,x27 ; LOADU4
	add x26,x26,x30
L.694:
	addi x27,x27,1
	li x30,10000
	blt x27,x30,L.693
	la x30,Devices
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,43
	call _sbd
	mv x27,x0 ; LOADI4
L.697:
	mv x30,x27 ; LOADU4
	add x26,x26,x30
L.698:
	addi x27,x27,1
	li x30,1000000
	blt x27,x30,L.697
L.684:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl ser
	.align	4
ser:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	j L.703
L.702:
	la x12,KeyboardEvents
	call kbd_event_pop
	mv x27,x10 ; LOADP4
	mv x30,x27 ; LOADU4
	beq x30,x0,L.705
	li x30,2
	add x30,x27,x30
	lbu x30,0(x30)
	beq x30,x0,L.707
	la x30,Devices
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x30,2
	add x30,x27,x30
	lbu x13,0(x30)
	call _sbd
L.707:
	lhu x30,0(x27)
	li x29,256
	bne x30,x29,L.709
	j L.704
L.709:
L.705:
	call serial_available
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.711
	la x30,Devices
	lbu x30,0(x30)
	addi x30,x30,3
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	mv x25,x30 ; LOADU1
	mv x26,x0 ; LOADU1
	j L.716
L.713:
	la x30,Devices
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	la x12,Video_tty
	mv x30,x10 ; LOADU4
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	call ttty_putc
L.714:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	mv x26,x30 ; LOADU1
L.716:
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	shlli x29,x25,8*(4-1)
	shrli x29,x29,8*(4-1)
	blt x30,x29,L.713
L.711:
L.703:
	j L.702
L.704:
L.701:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align	4
synth_write:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	la x30,Devices+6
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0+16(x8)
	call _sbd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,4+16(x8)
	call _sbd
	mv x10,x0 ; LOADI4
L.717:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
L.721:
	.byte	0x5
	.align 4
	.text
	.align	4
vol_control:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x12,KeyboardEvents
	call kbd_event_pop
	sw x10,-8+16(x8)
	lw x30,-8+16(x8)
	beq x30,x0,L.722
	lw x30,-8+16(x8)
	lhu x30,0(x30)
	li x29,256
	bne x30,x29,L.724
	li x10,1
	j L.720
L.724:
	lw x30,-8+16(x8)
	lhu x30,0(x30)
	li x29,265
	bne x30,x29,L.726
	la x30,L.721
	lbu x30,0(x30)
	li x29,15
	bge x30,x29,L.726
	la x30,L.721
	lbu x29,0(x30)
	addi x29,x29,1
	sb x29,0(x30)
	la x29,Devices+6
	lbu x29,0(x29)
	addi x29,x29,12
	shlli x12,x29,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0(x30)
	call _sbd
	j L.727
L.726:
	lw x30,-8+16(x8)
	lhu x30,0(x30)
	li x29,264
	bne x30,x29,L.729
	la x30,L.721
	lbu x30,0(x30)
	ble x30,x0,L.729
	la x30,L.721
	lbu x29,0(x30)
	addi x29,x29,-1
	sb x29,0(x30)
	la x29,Devices+6
	lbu x29,0(x29)
	addi x29,x29,12
	shlli x12,x29,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0(x30)
	call _sbd
L.729:
L.727:
L.722:
	mv x10,x0 ; LOADI4
L.720:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl play_chord
	.align	4
play_chord:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	lw x30,0+16(x8)
	sh x30,0+16(x8)
	lw x30,4+16(x8)
	sh x30,4+16(x8)
	lw x30,8+16(x8)
	sh x30,8+16(x8)
	lhu x30,0+16(x8)
	sh x30,-10+16(x8)
	lhu x30,4+16(x8)
	sh x30,-8+16(x8)
	lhu x30,8+16(x8)
	sh x30,-6+16(x8)
	mv x27,x0 ; LOADI4
L.735:
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,7
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x30,x27 ; LOADU4
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	call _sbd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,3
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x30,1
	shll x30,x27,x30
	addi x29,x8,-10+16
	add x30,x30,x29
	lhu x13,0(x30)
	call _shd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,800
	call _shd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,2
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
L.736:
	addi x27,x27,1
	li x30,3
	blt x27,x30,L.735
L.743:
	call vol_control
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.746
	j L.732
L.746:
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lwd
	mv x26,x10 ; LOADU4
L.744:
	li x30,7
	and x29,x26,x30
	bne x29,x30,L.743
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,7
	call _sbd
L.732:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl fanfare
	.align	4
fanfare:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	li x12,48
	li x13,112
	call synth_write
	li x12,49
	li x13,114
	call synth_write
	li x12,50
	li x13,148
	call synth_write
	j L.752
L.751:
	call vol_control
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.754
	j L.753
L.754:
	li x12,2320
	li x13,2391
	li x14,2455
	call play_chord
	li x12,2411
	li x13,2505
	li x14,2832
	call play_chord
	li x12,1943
	li x13,2032
	li x14,2354
	call play_chord
L.752:
	j L.751
L.753:
L.750:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	2
L.757:
	.half	0x0
	.half	0x1fd5
	.half	0x3e2e
	.half	0x59ae
	.half	0x711f
	.half	0x7fff
	.half	0x7fff
	.half	0x7fff
	.half	0x711f
	.half	0x59ae
	.half	0x3e2e
	.half	0x1fd5
	.half	0x0
	.half	0xe02b
	.half	0xc1d2
	.half	0xa652
	.half	0x8ee1
	.half	0x8001
	.half	0x8001
	.half	0x8001
	.half	0x8ee1
	.half	0xa652
	.half	0xc1d2
	.half	0xe02b
	.globl pcm
	.align 4
	.text
	.align	4
pcm:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x27,x0 ; LOADU1
	j L.759
L.758:
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lwd
	mv x26,x10 ; LOADU4
	call vol_control
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.762
	j L.760
L.762:
	li x30,0x100000
	and x30,x26,x30
	bne x30,x0,L.764
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,13
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,1
	shll x30,x30,x29
	la x29,L.757
	add x30,x30,x29
	lh x30,0(x30)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	shlli x30,x27,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	li x29,24
	idiv x0,x30,x30,x29
	mv x27,x30 ; LOADU1
L.764:
L.759:
	j L.758
L.760:
L.756:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.data
	.align	2
L.768:
	.half	0x0
	.half	0x1fd5
	.half	0x3e2e
	.half	0x59ae
	.half	0x711f
	.half	0x7fff
	.half	0x7fff
	.half	0x7fff
	.half	0x711f
	.half	0x59ae
	.half	0x3e2e
	.half	0x1fd5
	.half	0x0
	.half	0xe02b
	.half	0xc1d2
	.half	0xa652
	.half	0x8ee1
	.half	0x8001
	.half	0x8001
	.half	0x8001
	.half	0x8ee1
	.half	0xa652
	.half	0xc1d2
	.half	0xe02b
	.globl pcm_synth
	.align 4
	.text
	.align	4
pcm_synth:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x26,x0 ; LOADU1
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,7
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,3
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,2505
	call _shd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,800
	call _shd
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,2
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
L.773:
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lwd
	mv x27,x10 ; LOADU4
	call vol_control
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.777
	j L.775
L.777:
	li x30,0x100000
	and x30,x27,x30
	bne x30,x0,L.779
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,13
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,1
	shll x30,x30,x29
	la x29,L.768
	add x30,x30,x29
	lh x30,0(x30)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	shlli x30,x26,8*(4-1)
	shrli x30,x30,8*(4-1)
	addi x30,x30,1
	li x29,24
	idiv x0,x30,x30,x29
	mv x26,x30 ; LOADU1
L.779:
L.774:
	li x30,1
	and x30,x27,x30
	bne x30,x0,L.773
L.775:
L.767:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl tamlin
	.align	4
tamlin:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x24,28(x2)
	sw x25,32(x2)
	sw x26,36(x2)
	sw x27,40(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	li x25,0x1000
	li x26,0x14fbf
	mv x27,x0 ; LOADU4
	lh x30,0(x25)
	li x29,-7484
	beq x30,x29,L.787
	la x12,Con
	la x13,L.785
	call ttty_mux_printf
	j L.782
L.786:
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lwd
	mv x24,x10 ; LOADU4
	call vol_control
	mv x30,x10 ; LOADI1
	shlli x30,x30,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.790
	j L.788
L.790:
	li x30,0x100000
	and x30,x24,x30
	bne x30,x0,L.792
	la x30,Devices+6
	lbu x30,0(x30)
	addi x30,x30,13
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x30,1
	shll x30,x27,x30
	add x30,x30,x25
	lh x30,0(x30)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	li x30,1
	add x27,x27,x30
L.792:
L.787:
	bltu x27,x26,L.786
L.788:
L.782:
	lw x1,24(x2)
	lw x24,28(x2)
	lw x25,32(x2)
	lw x26,36(x2)
	lw x27,40(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.data
	.align	1
video_context:
	.byte	0x0
	.align	1
video_is_drawing:
	.byte	0x0
	.align 4
	.text
	.align	4
video_begin_drawing:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	la x30,video_context
	lbu x29,0+16(x8)
	sb x29,0(x30)
	la x30,video_is_drawing
	li x29,1
	sb x29,0(x30)
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,2
	call _sbd
L.795:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
video_end_drawing:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,video_is_drawing
	sb x0,0(x30)
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
L.797:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
video_clear:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	lw x30,4+16(x8)
	sb x30,4+16(x8)
	la x30,video_is_drawing
	lb x30,0(x30)
	bne x30,x0,L.800
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,2
	call _sbd
L.800:
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,0x20000004
	call _swd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0+16(x8)
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,6
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,4+16(x8)
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,5
	call _sbd
	la x30,video_is_drawing
	lb x30,0(x30)
	bne x30,x0,L.808
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,1
	call _sbd
L.808:
L.799:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align	4
draw_line:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	sw x15,28(x8)
	sw x16,32(x8)
	lw x30,0+16(x8)
	sb x30,0+16(x8)
	lw x30,4+16(x8)
	sh x30,4+16(x8)
	lw x30,8+16(x8)
	sh x30,8+16(x8)
	lw x30,12+16(x8)
	sh x30,12+16(x8)
	lw x30,16+16(x8)
	sh x30,16+16(x8)
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	la x30,video_context
	lbu x13,0(x30)
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,2
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lbu x13,0+16(x8)
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,5
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lh x30,4+16(x8)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,7
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lh x30,8+16(x8)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,1
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lh x30,12+16(x8)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,3
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	lh x30,16+16(x8)
	shlli x13,x30,8*(4-2)
	shrli x13,x13,8*(4-2)
	call _shd
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,8
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x13,x0 ; LOADI4
	call _sbd
	la x30,Devices+3
	lbu x30,0(x30)
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	li x13,13
	call _sbd
L.811:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.821:
	.word	0x640000
	.align	4
L.822:
	.word	0x640000
	.align	4
L.823:
	.word	0x249f0
	.align	4
L.824:
	.word	0x1d4c0
	.globl bouncing_box
	.align 4
	.text
	.align	4
bouncing_box:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x23,28(x2)
	sw x24,32(x2)
	sw x25,36(x2)
	sw x26,40(x2)
	sw x27,44(x2)
	li x25,80
	la x30,L.821
	lw x29,0(x30)
	la x28,L.823
	lw x28,0(x28)
	add x29,x29,x28
	sw x29,0(x30)
	la x29,L.822
	lw x28,0(x29)
	la x7,L.824
	lw x7,0(x7)
	add x28,x28,x7
	sw x28,0(x29)
	li x28,16
	lw x30,0(x30)
	shra x30,x30,x28
	mv x27,x30 ; LOADI2
	lw x30,0(x29)
	shra x30,x30,x28
	mv x26,x30 ; LOADI2
	shlli x30,x27,8*(4-2)
	shrai x30,x30,8*(4-2)
	li x29,-40
	blt x30,x29,L.827
	li x29,600
	ble x30,x29,L.825
L.827:
	la x30,L.823
	lw x29,0(x30)
	not x29,x29
	li x28,1
	add x29,x29,x28
	sw x29,0(x30)
L.825:
	shlli x30,x26,8*(4-2)
	shrai x30,x30,8*(4-2)
	li x29,-40
	blt x30,x29,L.830
	li x29,440
	ble x30,x29,L.828
L.830:
	la x30,L.824
	lw x29,0(x30)
	not x29,x29
	li x28,1
	add x29,x29,x28
	sw x29,0(x30)
L.828:
	mv x12,x0 ; LOADI4
	call video_begin_drawing
	li x12,5
	li x13,2
	call video_clear
	li x12,255
	mv x13,x0 ; LOADI4
	li x30,240
	mv x14,x30 ; LOADI4
	li x15,639
	mv x16,x30 ; LOADI4
	call draw_line
	li x12,255
	li x30,320
	mv x13,x30 ; LOADI4
	mv x14,x0 ; LOADI4
	mv x15,x30 ; LOADI4
	li x16,479
	call draw_line
	shlli x30,x27,8*(4-2)
	shrai x30,x30,8*(4-2)
	shlli x29,x25,8*(4-2)
	shrli x29,x29,8*(4-2)
	add x28,x30,x29
	mv x24,x28 ; LOADI2
	shlli x28,x26,8*(4-2)
	shrai x28,x28,8*(4-2)
	add x29,x28,x29
	mv x23,x29 ; LOADI2
	li x12,11
	mv x13,x30 ; LOADI4
	mv x14,x28 ; LOADI4
	shlli x15,x24,8*(4-2)
	shrai x15,x15,8*(4-2)
	mv x16,x28 ; LOADI4
	call draw_line
	li x12,11
	shlli x30,x24,8*(4-2)
	shrai x30,x30,8*(4-2)
	mv x13,x30 ; LOADI4
	shlli x14,x26,8*(4-2)
	shrai x14,x14,8*(4-2)
	mv x15,x30 ; LOADI4
	shlli x16,x23,8*(4-2)
	shrai x16,x16,8*(4-2)
	call draw_line
	li x12,11
	shlli x13,x24,8*(4-2)
	shrai x13,x13,8*(4-2)
	shlli x30,x23,8*(4-2)
	shrai x30,x30,8*(4-2)
	mv x14,x30 ; LOADI4
	shlli x15,x27,8*(4-2)
	shrai x15,x15,8*(4-2)
	mv x16,x30 ; LOADI4
	call draw_line
	li x12,11
	shlli x30,x27,8*(4-2)
	shrai x30,x30,8*(4-2)
	mv x13,x30 ; LOADI4
	shlli x14,x23,8*(4-2)
	shrai x14,x14,8*(4-2)
	mv x15,x30 ; LOADI4
	shlli x16,x26,8*(4-2)
	shrai x16,x16,8*(4-2)
	call draw_line
	call video_end_drawing
	la x12,Video_tty
	la x13,L.831
	shlli x30,x25,8*(4-2)
	shrli x30,x30,8*(4-2)
	li x29,2
	idiv x30,x0,x30,x29
	shlli x29,x27,8*(4-2)
	shrai x29,x29,8*(4-2)
	add x14,x29,x30
	shlli x29,x26,8*(4-2)
	shrai x29,x29,8*(4-2)
	add x15,x29,x30
	call ttty_printf
L.820:
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
	.align	4
on_vblank:
	.word	0x0
	.globl test2d
	.align 4
	.text
	.align	4
test2d:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x26,28(x2)
	sw x27,32(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	mv x27,x0 ; LOADI1
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	mv x26,x30 ; LOADU4
	mv x12,x0 ; LOADI4
	li x13,3
	call video_clear
	la x30,on_vblank
	la x29,bouncing_box
	sw x29,0(x30)
	li x30,1
	or x26,x26,x30
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x30,x26 ; LOADU1
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	call _sbd
L.835:
	call vol_control
	mv x27,x10 ; LOADI1
L.836:
	shlli x30,x27,8*(4-1)
	shrai x30,x30,8*(4-1)
	beq x30,x0,L.835
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	call _lbud
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	mv x26,x30 ; LOADU4
	li x30,-2
	and x26,x26,x30
	la x30,Devices+3
	lbu x30,0(x30)
	addi x30,x30,14
	shlli x12,x30,8*(4-2)
	shrli x12,x12,8*(4-2)
	mv x30,x26 ; LOADU1
	shlli x13,x30,8*(4-1)
	shrli x13,x13,8*(4-1)
	call _sbd
	mv x12,x0 ; LOADI4
	li x13,3
	call video_clear
	la x30,on_vblank
	sw x0,0(x30)
	la x12,Video_tty
	call ttty_clear
L.832:
	lw x1,24(x2)
	lw x26,28(x2)
	lw x27,32(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl bios_vblank_handler
	.align	4
bios_vblank_handler:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	la x30,on_vblank
	lw x30,0(x30)
	beq x30,x0,L.841
	la x30,on_vblank
	lw x31,0(x30)
	jalr x1, 0(x31)
L.841:
L.840:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
commands:
	.word	L.843
	.word	L.844
	.word	sysinfo
	.word	L.845
	.word	L.846
	.word	echo
	.word	L.847
	.word	L.848
	.word	peek
	.word	L.849
	.word	L.850
	.word	poke
	.word	L.851
	.word	L.852
	.word	ser
	.word	L.853
	.word	L.854
	.word	send_escape
	.word	L.855
	.word	L.856
	.word	fanfare
	.word	L.857
	.word	L.858
	.word	pcm
	.word	L.859
	.word	L.860
	.word	pcm_synth
	.word	L.861
	.word	L.862
	.word	tamlin
	.word	L.863
	.word	L.864
	.word	test2d
	.word	L.865
	.word	L.866
	.word	clear
	.globl bios_start
	.align 4
	.text
	.align	4
bios_start:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x25,28(x2)
	sw x26,32(x2)
	sw x27,36(x2)
	call bios_init
	mv x27,x0 ; LOADU4
	j L.871
L.868:
	add x26,x26,x27
L.869:
	li x30,1
	add x27,x27,x30
L.871:
	li x30,0x186a0
	bltu x27,x30,L.868
	la x12,Con
	call ttty_mux_clear
	la x12,Con
	la x13,L.872
	la x14,L.873
	la x15,L.874
	li x16,84
	call ttty_mux_printf
	la x12,Video_tty
	la x13,L.875
	la x30,Video_tty+4
	lbu x14,0(x30)
	la x30,Video_tty+5
	lbu x15,0(x30)
	la x30,Video_tty+8
	lbu x16,0(x30)
	call ttty_printf
	la x12,commands
	li x13,12
	call ushell_init
	j L.880
L.879:
	la x12,Keyboard_in
	call tins_getc
	mv x30,x10 ; LOADU4
	mv x25,x30 ; LOADU1
	shlli x12,x25,8*(4-1)
	shrli x12,x12,8*(4-1)
	call ushell_process
L.880:
	j L.879
L.867:
	lw x1,24(x2)
	lw x25,28(x2)
	lw x26,32(x2)
	lw x27,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.align 4
	.bss
	.align	1
command:
	.space	160
	.globl Keyboard_in
	.align	4
Keyboard_in:
	.space	16
	.globl KeyboardEvents
	.align	2
KeyboardEvents:
	.space	194
	.globl Charbuff
	.align	4
Charbuff:
	.space	4
	.globl Charbuff_size
	.align	4
Charbuff_size:
	.space	4
	.globl Stack_base
	.align	4
Stack_base:
	.space	4
	.globl Stack_top
	.align	4
Stack_top:
	.space	4
	.globl CurrentTps
	.align	4
CurrentTps:
	.space	4
	.globl Memsize
	.align	4
Memsize:
	.space	4
	.globl Serial_tty
	.align	4
Serial_tty:
	.space	76
	.globl Video_tty
	.align	4
Video_tty:
	.space	76
	.globl Con
	.align	4
Con:
	.space	40
	.globl Devices
	.align	1
Devices:
	.space	16
	.align 4
	.data
	.align	1
L.875:
	.byte	0x54
	.byte	0x65
	.byte	0x72
	.byte	0x6d
	.byte	0x69
	.byte	0x6e
	.byte	0x61
	.byte	0x6c
	.byte	0x20
	.byte	0x73
	.byte	0x69
	.byte	0x7a
	.byte	0x65
	.byte	0x3a
	.byte	0x20
	.byte	0x25
	.byte	0x64
	.byte	0x20
	.byte	0x63
	.byte	0x6f
	.byte	0x6c
	.byte	0x73
	.byte	0x2c
	.byte	0x20
	.byte	0x25
	.byte	0x64
	.byte	0x20
	.byte	0x72
	.byte	0x6f
	.byte	0x77
	.byte	0x73
	.byte	0x2c
	.byte	0x20
	.byte	0x25
	.byte	0x64
	.byte	0x20
	.byte	0x62
	.byte	0x70
	.byte	0x63
	.byte	0xa
	.byte	0x0
	.align	1
L.874:
	.byte	0x30
	.byte	0x2e
	.byte	0x35
	.byte	0x20
	.byte	0x62
	.byte	0x65
	.byte	0x74
	.byte	0x61
	.byte	0x0
	.align	1
L.873:
	.byte	0x42
	.byte	0x61
	.byte	0x73
	.byte	0x69
	.byte	0x63
	.byte	0x20
	.byte	0x42
	.byte	0x49
	.byte	0x4f
	.byte	0x53
	.byte	0x0
	.align	1
L.872:
	.byte	0x25
	.byte	0x73
	.byte	0x20
	.byte	0x25
	.byte	0x73
	.byte	0x20
	.byte	0x25
	.byte	0x63
	.byte	0xa
	.byte	0x0
	.align	1
L.866:
	.byte	0x63
	.byte	0x6c
	.byte	0x65
	.byte	0x61
	.byte	0x72
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x73
	.byte	0x63
	.byte	0x72
	.byte	0x65
	.byte	0x65
	.byte	0x6e
	.byte	0x0
	.align	1
L.865:
	.byte	0x63
	.byte	0x6c
	.byte	0x65
	.byte	0x61
	.byte	0x72
	.byte	0x0
	.align	1
L.864:
	.byte	0x61
	.byte	0x20
	.byte	0x76
	.byte	0x65
	.byte	0x72
	.byte	0x79
	.byte	0x20
	.byte	0x73
	.byte	0x69
	.byte	0x6d
	.byte	0x70
	.byte	0x6c
	.byte	0x65
	.byte	0x20
	.byte	0x32
	.byte	0x64
	.byte	0x20
	.byte	0x64
	.byte	0x65
	.byte	0x6d
	.byte	0x6f
	.byte	0x0
	.align	1
L.863:
	.byte	0x62
	.byte	0x6f
	.byte	0x78
	.byte	0x0
	.align	1
L.862:
	.byte	0x70
	.byte	0x6c
	.byte	0x61
	.byte	0x79
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x62
	.byte	0x69
	.byte	0x74
	.byte	0x20
	.byte	0x6f
	.byte	0x66
	.byte	0x20
	.byte	0x54
	.byte	0x61
	.byte	0x6d
	.byte	0x20
	.byte	0x4c
	.byte	0x69
	.byte	0x6e
	.byte	0x0
	.align	1
L.861:
	.byte	0x74
	.byte	0x61
	.byte	0x6d
	.byte	0x6c
	.byte	0x69
	.byte	0x6e
	.byte	0x0
	.align	1
L.860:
	.byte	0x70
	.byte	0x6c
	.byte	0x61
	.byte	0x79
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x70
	.byte	0x63
	.byte	0x6d
	.byte	0x20
	.byte	0x73
	.byte	0x69
	.byte	0x6e
	.byte	0x65
	.byte	0x20
	.byte	0x77
	.byte	0x61
	.byte	0x76
	.byte	0x65
	.byte	0x20
	.byte	0x61
	.byte	0x6e
	.byte	0x64
	.byte	0x20
	.byte	0x41
	.byte	0x34
	.byte	0x0
	.align	1
L.859:
	.byte	0x61
	.byte	0x75
	.byte	0x64
	.byte	0x69
	.byte	0x6f
	.byte	0x0
	.align	1
L.858:
	.byte	0x70
	.byte	0x6c
	.byte	0x61
	.byte	0x79
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x70
	.byte	0x63
	.byte	0x6d
	.byte	0x20
	.byte	0x73
	.byte	0x69
	.byte	0x6e
	.byte	0x65
	.byte	0x20
	.byte	0x77
	.byte	0x61
	.byte	0x76
	.byte	0x65
	.byte	0x0
	.align	1
L.857:
	.byte	0x70
	.byte	0x63
	.byte	0x6d
	.byte	0x0
	.align	1
L.856:
	.byte	0x70
	.byte	0x6c
	.byte	0x61
	.byte	0x79
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x73
	.byte	0x69
	.byte	0x6d
	.byte	0x70
	.byte	0x6c
	.byte	0x65
	.byte	0x20
	.byte	0x66
	.byte	0x61
	.byte	0x6e
	.byte	0x66
	.byte	0x61
	.byte	0x72
	.byte	0x65
	.byte	0x2e
	.byte	0x20
	.byte	0x65
	.byte	0x73
	.byte	0x63
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x71
	.byte	0x75
	.byte	0x69
	.byte	0x74
	.byte	0x0
	.align	1
L.855:
	.byte	0x66
	.byte	0x61
	.byte	0x6e
	.byte	0x66
	.byte	0x61
	.byte	0x72
	.byte	0x65
	.byte	0x0
	.align	1
L.854:
	.byte	0x74
	.byte	0x65
	.byte	0x73
	.byte	0x74
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x41
	.byte	0x54
	.byte	0x20
	.byte	0x65
	.byte	0x73
	.byte	0x63
	.byte	0x61
	.byte	0x70
	.byte	0x65
	.byte	0x20
	.byte	0x73
	.byte	0x65
	.byte	0x63
	.byte	0x75
	.byte	0x65
	.byte	0x6e
	.byte	0x63
	.byte	0x65
	.byte	0x0
	.align	1
L.853:
	.byte	0x65
	.byte	0x73
	.byte	0x63
	.byte	0x61
	.byte	0x70
	.byte	0x65
	.byte	0x0
	.align	1
L.852:
	.byte	0x64
	.byte	0x72
	.byte	0x6f
	.byte	0x70
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x73
	.byte	0x65
	.byte	0x72
	.byte	0x69
	.byte	0x61
	.byte	0x6c
	.byte	0x20
	.byte	0x74
	.byte	0x65
	.byte	0x72
	.byte	0x6d
	.byte	0x69
	.byte	0x6e
	.byte	0x61
	.byte	0x6c
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x6d
	.byte	0x6f
	.byte	0x64
	.byte	0x65
	.byte	0x6d
	.byte	0x0
	.align	1
L.851:
	.byte	0x73
	.byte	0x65
	.byte	0x72
	.byte	0x0
	.align	1
L.850:
	.byte	0x73
	.byte	0x65
	.byte	0x74
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x76
	.byte	0x61
	.byte	0x6c
	.byte	0x75
	.byte	0x65
	.byte	0x20
	.byte	0x69
	.byte	0x6e
	.byte	0x20
	.byte	0x6d
	.byte	0x65
	.byte	0x6d
	.byte	0x6f
	.byte	0x72
	.byte	0x79
	.byte	0x0
	.align	1
L.849:
	.byte	0x70
	.byte	0x6f
	.byte	0x6b
	.byte	0x65
	.byte	0x0
	.align	1
L.848:
	.byte	0x70
	.byte	0x72
	.byte	0x69
	.byte	0x6e
	.byte	0x74
	.byte	0x20
	.byte	0x61
	.byte	0x20
	.byte	0x6d
	.byte	0x65
	.byte	0x6d
	.byte	0x6f
	.byte	0x72
	.byte	0x79
	.byte	0x20
	.byte	0x64
	.byte	0x75
	.byte	0x6d
	.byte	0x70
	.byte	0x0
	.align	1
L.847:
	.byte	0x70
	.byte	0x65
	.byte	0x65
	.byte	0x6b
	.byte	0x0
	.align	1
L.846:
	.byte	0x65
	.byte	0x63
	.byte	0x68
	.byte	0x6f
	.byte	0x65
	.byte	0x73
	.byte	0x20
	.byte	0x69
	.byte	0x74
	.byte	0x73
	.byte	0x20
	.byte	0x61
	.byte	0x72
	.byte	0x67
	.byte	0x75
	.byte	0x6d
	.byte	0x65
	.byte	0x6e
	.byte	0x74
	.byte	0x73
	.byte	0x0
	.align	1
L.845:
	.byte	0x65
	.byte	0x63
	.byte	0x68
	.byte	0x6f
	.byte	0x0
	.align	1
L.844:
	.byte	0x70
	.byte	0x72
	.byte	0x69
	.byte	0x6e
	.byte	0x74
	.byte	0x73
	.byte	0x20
	.byte	0x73
	.byte	0x79
	.byte	0x73
	.byte	0x74
	.byte	0x65
	.byte	0x6d
	.byte	0x20
	.byte	0x69
	.byte	0x6e
	.byte	0x66
	.byte	0x6f
	.byte	0x72
	.byte	0x6d
	.byte	0x61
	.byte	0x74
	.byte	0x69
	.byte	0x6f
	.byte	0x6e
	.byte	0x0
	.align	1
L.843:
	.byte	0x73
	.byte	0x79
	.byte	0x73
	.byte	0x69
	.byte	0x6e
	.byte	0x66
	.byte	0x6f
	.byte	0x0
	.align	1
L.831:
	.byte	0x1b
	.byte	0x5b
	.byte	0x33
	.byte	0x30
	.byte	0x3b
	.byte	0x36
	.byte	0x31
	.byte	0x48
	.byte	0x42
	.byte	0x6f
	.byte	0x78
	.byte	0x20
	.byte	0x7b
	.byte	0x78
	.byte	0x3a
	.byte	0x20
	.byte	0x25
	.byte	0x30
	.byte	0x33
	.byte	0x64
	.byte	0x2c
	.byte	0x20
	.byte	0x79
	.byte	0x3a
	.byte	0x20
	.byte	0x25
	.byte	0x30
	.byte	0x33
	.byte	0x64
	.byte	0x7d
	.byte	0x0
	.align	1
L.785:
	.byte	0x54
	.byte	0x61
	.byte	0x6d
	.byte	0x6c
	.byte	0x69
	.byte	0x6e
	.byte	0x20
	.byte	0x69
	.byte	0x73
	.byte	0x20
	.byte	0x6e
	.byte	0x6f
	.byte	0x74
	.byte	0x20
	.byte	0x6c
	.byte	0x6f
	.byte	0x61
	.byte	0x64
	.byte	0x65
	.byte	0x64
	.byte	0x2c
	.byte	0x20
	.byte	0x6f
	.byte	0x72
	.byte	0x20
	.byte	0x64
	.byte	0x61
	.byte	0x74
	.byte	0x61
	.byte	0x20
	.byte	0x69
	.byte	0x73
	.byte	0x20
	.byte	0x63
	.byte	0x6f
	.byte	0x72
	.byte	0x72
	.byte	0x75
	.byte	0x70
	.byte	0x74
	.byte	0x65
	.byte	0x64
	.byte	0xa
	.byte	0x28
	.byte	0x79
	.byte	0x6f
	.byte	0x75
	.byte	0x20
	.byte	0x6d
	.byte	0x75
	.byte	0x73
	.byte	0x74
	.byte	0x20
	.byte	0x6c
	.byte	0x6f
	.byte	0x61
	.byte	0x64
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x74
	.byte	0x61
	.byte	0x6d
	.byte	0x6c
	.byte	0x69
	.byte	0x6e
	.byte	0x2e
	.byte	0x72
	.byte	0x61
	.byte	0x77
	.byte	0x20
	.byte	0x66
	.byte	0x69
	.byte	0x6c
	.byte	0x65
	.byte	0x20
	.byte	0x77
	.byte	0x69
	.byte	0x74
	.byte	0x68
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x65
	.byte	0x6d
	.byte	0x75
	.byte	0x6c
	.byte	0x61
	.byte	0x74
	.byte	0x6f
	.byte	0x72
	.byte	0x29
	.byte	0x0
	.align	1
L.683:
	.byte	0x1b
	.byte	0x5b
	.byte	0x32
	.byte	0x4a
	.byte	0x1b
	.byte	0x5b
	.byte	0x31
	.byte	0x3b
	.byte	0x31
	.byte	0x48
	.byte	0x0
	.align	1
L.681:
	.byte	0x9
	.byte	0x3c
	.byte	0x76
	.byte	0x61
	.byte	0x6c
	.byte	0x75
	.byte	0x65
	.byte	0x3e
	.byte	0x9
	.byte	0x76
	.byte	0x61
	.byte	0x6c
	.byte	0x75
	.byte	0x65
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x73
	.byte	0x65
	.byte	0x74
	.byte	0x20
	.byte	0x28
	.byte	0x61
	.byte	0x20
	.byte	0x62
	.byte	0x79
	.byte	0x74
	.byte	0x65
	.byte	0x29
	.byte	0xa
	.byte	0x0
	.align	1
L.680:
	.byte	0x9
	.byte	0x3c
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x3e
	.byte	0x9
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x20
	.byte	0x69
	.byte	0x6e
	.byte	0x20
	.byte	0x68
	.byte	0x65
	.byte	0x78
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x70
	.byte	0x6f
	.byte	0x6b
	.byte	0x65
	.byte	0xa
	.byte	0x0
	.align	1
L.679:
	.byte	0x55
	.byte	0x73
	.byte	0x61
	.byte	0x67
	.byte	0x65
	.byte	0x3a
	.byte	0x20
	.byte	0x70
	.byte	0x6f
	.byte	0x6b
	.byte	0x65
	.byte	0x20
	.byte	0x3c
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x3e
	.byte	0x20
	.byte	0x3c
	.byte	0x76
	.byte	0x61
	.byte	0x6c
	.byte	0x75
	.byte	0x65
	.byte	0x3e
	.byte	0xa
	.byte	0x0
	.align	1
L.666:
	.byte	0x7c
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x0
	.align	1
L.665:
	.byte	0x9
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x0
	.align	1
L.664:
	.byte	0x25
	.byte	0x78
	.byte	0x3a
	.byte	0x0
	.align	1
L.663:
	.byte	0x25
	.byte	0x78
	.byte	0x20
	.byte	0x2d
	.byte	0x3e
	.byte	0x20
	.byte	0x25
	.byte	0x78
	.byte	0x0
	.align	1
L.658:
	.byte	0x9
	.byte	0x3c
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x3e
	.byte	0x9
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x20
	.byte	0x69
	.byte	0x6e
	.byte	0x20
	.byte	0x68
	.byte	0x65
	.byte	0x78
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x70
	.byte	0x65
	.byte	0x65
	.byte	0x6b
	.byte	0x20
	.byte	0x28
	.byte	0x31
	.byte	0x32
	.byte	0x38
	.byte	0x20
	.byte	0x62
	.byte	0x79
	.byte	0x74
	.byte	0x65
	.byte	0x73
	.byte	0x29
	.byte	0xa
	.byte	0x0
	.align	1
L.657:
	.byte	0x55
	.byte	0x73
	.byte	0x61
	.byte	0x67
	.byte	0x65
	.byte	0x3a
	.byte	0x20
	.byte	0x70
	.byte	0x65
	.byte	0x65
	.byte	0x6b
	.byte	0x20
	.byte	0x3c
	.byte	0x61
	.byte	0x64
	.byte	0x64
	.byte	0x72
	.byte	0x3e
	.byte	0x20
	.byte	0xa
	.byte	0x0
	.align	1
L.580:
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
	.byte	0x61
	.byte	0x62
	.byte	0x63
	.byte	0x64
	.byte	0x65
	.byte	0x66
	.byte	0x67
	.byte	0x68
	.byte	0x69
	.byte	0x6a
	.byte	0x6b
	.byte	0x6c
	.byte	0x6d
	.byte	0x6e
	.byte	0x6f
	.byte	0x70
	.byte	0x71
	.byte	0x72
	.byte	0x73
	.byte	0x74
	.byte	0x75
	.byte	0x76
	.byte	0x77
	.byte	0x78
	.byte	0x79
	.byte	0x7a
	.byte	0x0
	.align	1
L.533:
	.byte	0x1b
	.byte	0x25
	.byte	0x73
	.byte	0x0
	.align	1
L.527:
	.byte	0x9
	.byte	0x2d
	.byte	0x65
	.byte	0x9
	.byte	0x65
	.byte	0x63
	.byte	0x68
	.byte	0x6f
	.byte	0x20
	.byte	0x61
	.byte	0x72
	.byte	0x67
	.byte	0x73
	.byte	0x20
	.byte	0x77
	.byte	0x69
	.byte	0x74
	.byte	0x68
	.byte	0x6f
	.byte	0x75
	.byte	0x74
	.byte	0x20
	.byte	0x70
	.byte	0x72
	.byte	0x6f
	.byte	0x63
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x69
	.byte	0x6e
	.byte	0x67
	.byte	0xa
	.byte	0x0
	.align	1
L.526:
	.byte	0x9
	.byte	0x2d
	.byte	0x61
	.byte	0x9
	.byte	0x70
	.byte	0x72
	.byte	0x65
	.byte	0x70
	.byte	0x65
	.byte	0x6e
	.byte	0x64
	.byte	0x20
	.byte	0x5c
	.byte	0x78
	.byte	0x31
	.byte	0x62
	.byte	0x20
	.byte	0x28
	.byte	0x45
	.byte	0x53
	.byte	0x43
	.byte	0x29
	.byte	0x20
	.byte	0x74
	.byte	0x6f
	.byte	0x20
	.byte	0x70
	.byte	0x72
	.byte	0x6f
	.byte	0x63
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x20
	.byte	0x41
	.byte	0x4e
	.byte	0x53
	.byte	0x49
	.byte	0x20
	.byte	0x65
	.byte	0x73
	.byte	0x63
	.byte	0x61
	.byte	0x70
	.byte	0x65
	.byte	0x20
	.byte	0x63
	.byte	0x6f
	.byte	0x64
	.byte	0x65
	.byte	0xa
	.byte	0x0
	.align	1
L.525:
	.byte	0x9
	.byte	0x2d
	.byte	0x68
	.byte	0x9
	.byte	0x70
	.byte	0x72
	.byte	0x69
	.byte	0x6e
	.byte	0x74
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x69
	.byte	0x73
	.byte	0x20
	.byte	0x68
	.byte	0x65
	.byte	0x6c
	.byte	0x70
	.byte	0xa
	.byte	0x0
	.align	1
L.524:
	.byte	0x55
	.byte	0x73
	.byte	0x61
	.byte	0x67
	.byte	0x65
	.byte	0x3a
	.byte	0x20
	.byte	0x65
	.byte	0x63
	.byte	0x68
	.byte	0x6f
	.byte	0x20
	.byte	0x2d
	.byte	0x5b
	.byte	0x68
	.byte	0x7c
	.byte	0x61
	.byte	0x7c
	.byte	0x65
	.byte	0x5d
	.byte	0x20
	.byte	0x3c
	.byte	0x61
	.byte	0x72
	.byte	0x67
	.byte	0x73
	.byte	0x3e
	.byte	0xa
	.byte	0x0
	.align	1
L.514:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x50
	.byte	0x6f
	.byte	0x69
	.byte	0x6e
	.byte	0x74
	.byte	0x65
	.byte	0x72
	.byte	0x20
	.byte	0x28
	.byte	0x6d
	.byte	0x6f
	.byte	0x75
	.byte	0x73
	.byte	0x65
	.byte	0x29
	.byte	0xa
	.byte	0x0
	.align	1
L.512:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x41
	.byte	0x75
	.byte	0x64
	.byte	0x69
	.byte	0x6f
	.byte	0x20
	.byte	0x53
	.byte	0x79
	.byte	0x6e
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x73
	.byte	0x69
	.byte	0x7a
	.byte	0x65
	.byte	0x72
	.byte	0xa
	.byte	0x0
	.align	1
L.510:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x48
	.byte	0x43
	.byte	0x53
	.byte	0x20
	.byte	0x43
	.byte	0x6f
	.byte	0x6e
	.byte	0x74
	.byte	0x72
	.byte	0x6f
	.byte	0x6c
	.byte	0x6c
	.byte	0x65
	.byte	0x72
	.byte	0xa
	.byte	0x0
	.align	1
L.508:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x54
	.byte	0x50
	.byte	0x53
	.byte	0x20
	.byte	0x43
	.byte	0x6f
	.byte	0x6e
	.byte	0x74
	.byte	0x72
	.byte	0x6f
	.byte	0x6c
	.byte	0x6c
	.byte	0x65
	.byte	0x72
	.byte	0xa
	.byte	0x0
	.align	1
L.506:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x47
	.byte	0x72
	.byte	0x61
	.byte	0x70
	.byte	0x68
	.byte	0x69
	.byte	0x63
	.byte	0x73
	.byte	0x20
	.byte	0x61
	.byte	0x6e
	.byte	0x64
	.byte	0x20
	.byte	0x56
	.byte	0x69
	.byte	0x64
	.byte	0x65
	.byte	0x6f
	.byte	0x20
	.byte	0x50
	.byte	0x72
	.byte	0x6f
	.byte	0x63
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x6f
	.byte	0x72
	.byte	0xa
	.byte	0x0
	.align	1
L.504:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x4b
	.byte	0x65
	.byte	0x79
	.byte	0x62
	.byte	0x6f
	.byte	0x61
	.byte	0x72
	.byte	0x64
	.byte	0xa
	.byte	0x0
	.align	1
L.502:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x44
	.byte	0x75
	.byte	0x61
	.byte	0x6c
	.byte	0x20
	.byte	0x54
	.byte	0x69
	.byte	0x6d
	.byte	0x65
	.byte	0x72
	.byte	0xa
	.byte	0x0
	.align	1
L.498:
	.byte	0x4e
	.byte	0x4f
	.byte	0x20
	.byte	0x43
	.byte	0x41
	.byte	0x52
	.byte	0x52
	.byte	0x49
	.byte	0x45
	.byte	0x52
	.byte	0x0
	.align	1
L.497:
	.byte	0x43
	.byte	0x41
	.byte	0x52
	.byte	0x52
	.byte	0x49
	.byte	0x45
	.byte	0x52
	.byte	0x20
	.byte	0x4f
	.byte	0x4b
	.byte	0x0
	.align	1
L.496:
	.byte	0x30
	.byte	0x78
	.byte	0x25
	.byte	0x78
	.byte	0x9
	.byte	0x25
	.byte	0x63
	.byte	0x9
	.byte	0x53
	.byte	0x65
	.byte	0x72
	.byte	0x69
	.byte	0x61
	.byte	0x6c
	.byte	0x20
	.byte	0x4d
	.byte	0x6f
	.byte	0x64
	.byte	0x65
	.byte	0x6d
	.byte	0x9
	.byte	0x5b
	.byte	0x20
	.byte	0x25
	.byte	0x73
	.byte	0x20
	.byte	0x5d
	.byte	0xa
	.byte	0x0
	.align	1
L.495:
	.byte	0x5b
	.byte	0x20
	.byte	0x44
	.byte	0x45
	.byte	0x56
	.byte	0x49
	.byte	0x43
	.byte	0x45
	.byte	0x53
	.byte	0x20
	.byte	0x5d
	.byte	0xa
	.byte	0x0
	.align	1
L.494:
	.byte	0x55
	.byte	0x70
	.byte	0x74
	.byte	0x69
	.byte	0x6d
	.byte	0x65
	.byte	0x3a
	.byte	0x9
	.byte	0x9
	.byte	0x25
	.byte	0x64
	.byte	0x20
	.byte	0x73
	.byte	0x65
	.byte	0x63
	.byte	0x6f
	.byte	0x6e
	.byte	0x64
	.byte	0x73
	.byte	0xa
	.byte	0xa
	.byte	0x0
	.align	1
L.493:
	.byte	0x46
	.byte	0x69
	.byte	0x72
	.byte	0x6d
	.byte	0x77
	.byte	0x61
	.byte	0x72
	.byte	0x65
	.byte	0x3a
	.byte	0x9
	.byte	0x42
	.byte	0x61
	.byte	0x73
	.byte	0x69
	.byte	0x63
	.byte	0x20
	.byte	0x42
	.byte	0x49
	.byte	0x4f
	.byte	0x53
	.byte	0x20
	.byte	0x76
	.byte	0x30
	.byte	0x2e
	.byte	0x35
	.byte	0x20
	.byte	0x62
	.byte	0x65
	.byte	0x74
	.byte	0x61
	.byte	0xa
	.byte	0x0
	.align	1
L.492:
	.byte	0x4d
	.byte	0x65
	.byte	0x6d
	.byte	0x6f
	.byte	0x72
	.byte	0x79
	.byte	0x3a
	.byte	0x9
	.byte	0x9
	.byte	0x25
	.byte	0x64
	.byte	0x4b
	.byte	0x69
	.byte	0x42
	.byte	0xa
	.byte	0x0
	.align	1
L.486:
	.byte	0x6f
	.byte	0x66
	.byte	0x20
	.byte	0x75
	.byte	0x6e
	.byte	0x6b
	.byte	0x6e
	.byte	0x6f
	.byte	0x77
	.byte	0x6e
	.byte	0x20
	.byte	0x6f
	.byte	0x72
	.byte	0x69
	.byte	0x67
	.byte	0x69
	.byte	0x6e
	.byte	0x0
	.align	1
L.485:
	.byte	0x6f
	.byte	0x66
	.byte	0x20
	.byte	0x74
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x48
	.byte	0x6f
	.byte	0x75
	.byte	0x73
	.byte	0x65
	.byte	0x20
	.byte	0x6f
	.byte	0x66
	.byte	0x20
	.byte	0x54
	.byte	0x61
	.byte	0x6c
	.byte	0x65
	.byte	0x61
	.byte	0x0
	.align	1
L.483:
	.byte	0x75
	.byte	0x6e
	.byte	0x6b
	.byte	0x6e
	.byte	0x6f
	.byte	0x77
	.byte	0x6e
	.byte	0x0
	.align	1
L.482:
	.byte	0x53
	.byte	0x69
	.byte	0x72
	.byte	0x69
	.byte	0x75
	.byte	0x73
	.byte	0x0
	.align	1
L.481:
	.byte	0x50
	.byte	0x72
	.byte	0x6f
	.byte	0x63
	.byte	0x65
	.byte	0x73
	.byte	0x73
	.byte	0x6f
	.byte	0x72
	.byte	0x3a
	.byte	0x9
	.byte	0x9
	.byte	0x25
	.byte	0x73
	.byte	0x20
	.byte	0x28
	.byte	0x25
	.byte	0x73
	.byte	0x29
	.byte	0x20
	.byte	0x40
	.byte	0x20
	.byte	0x25
	.byte	0x64
	.byte	0x4b
	.byte	0x68
	.byte	0x7a
	.byte	0xa
	.byte	0x0
	.align	1
L.480:
	.byte	0x5b
	.byte	0x20
	.byte	0x43
	.byte	0x4f
	.byte	0x52
	.byte	0x45
	.byte	0x20
	.byte	0x5d
	.byte	0xa
	.byte	0x0
	.align	1
L.479:
	.byte	0x54
	.byte	0x68
	.byte	0x65
	.byte	0x20
	.byte	0x54
	.byte	0x61
	.byte	0x6c
	.byte	0x65
	.byte	0x61
	.byte	0x20
	.byte	0x43
	.byte	0x6f
	.byte	0x6d
	.byte	0x70
	.byte	0x75
	.byte	0x74
	.byte	0x65
	.byte	0x72
	.byte	0x20
	.byte	0x53
	.byte	0x79
	.byte	0x73
	.byte	0x74
	.byte	0x65
	.byte	0x6d
	.byte	0x2e
	.byte	0x20
	.byte	0x76
	.byte	0x30
	.byte	0x2e
	.byte	0x35
	.byte	0x2d
	.byte	0x62
	.byte	0x65
	.byte	0x74
	.byte	0x61
	.byte	0xa
	.byte	0xa
	.byte	0x0
	.align	1
L.478:
	.byte	0x9
	.byte	0x9
	.byte	0x9
	.byte	0x53
	.byte	0x59
	.byte	0x53
	.byte	0x54
	.byte	0x45
	.byte	0x4d
	.byte	0x20
	.byte	0x49
	.byte	0x4e
	.byte	0x46
	.byte	0x4f
	.byte	0x52
	.byte	0x4d
	.byte	0x41
	.byte	0x54
	.byte	0x49
	.byte	0x4f
	.byte	0x4e
	.byte	0xa
	.byte	0x0
	.align	1
L.477:
	.byte	0x55
	.byte	0x73
	.byte	0x61
	.byte	0x67
	.byte	0x65
	.byte	0x3a
	.byte	0x20
	.byte	0x73
	.byte	0x79
	.byte	0x73
	.byte	0x69
	.byte	0x6e
	.byte	0x66
	.byte	0x6f
	.byte	0xa
	.byte	0x0
	.align	1
L.471:
	.byte	0x44
	.byte	0x75
	.byte	0x6d
	.byte	0x62
	.byte	0x20
	.byte	0x54
	.byte	0x65
	.byte	0x72
	.byte	0x6d
	.byte	0x69
	.byte	0x6e
	.byte	0x61
	.byte	0x6c
	.byte	0x0
	.align	1
L.470:
	.byte	0x53
	.byte	0x79
	.byte	0x73
	.byte	0x74
	.byte	0x65
	.byte	0x6d
	.byte	0x20
	.byte	0x43
	.byte	0x6f
	.byte	0x6e
	.byte	0x73
	.byte	0x6f
	.byte	0x6c
	.byte	0x65
	.byte	0x0
	.align	1
L.323:
	.byte	0x1b
	.byte	0x4f
	.byte	0x53
	.byte	0x0
	.align	1
L.322:
	.byte	0x1b
	.byte	0x4f
	.byte	0x52
	.byte	0x0
	.align	1
L.321:
	.byte	0x1b
	.byte	0x4f
	.byte	0x51
	.byte	0x0
	.align	1
L.320:
	.byte	0x1b
	.byte	0x4f
	.byte	0x50
	.byte	0x0
	.align	1
L.319:
	.byte	0x1b
	.byte	0x5b
	.byte	0x36
	.byte	0x7e
	.byte	0x0
	.align	1
L.318:
	.byte	0x1b
	.byte	0x5b
	.byte	0x35
	.byte	0x7e
	.byte	0x0
	.align	1
L.317:
	.byte	0x1b
	.byte	0x5b
	.byte	0x33
	.byte	0x7e
	.byte	0x0
	.align	1
L.316:
	.byte	0x1b
	.byte	0x5b
	.byte	0x46
	.byte	0x0
	.align	1
L.315:
	.byte	0x1b
	.byte	0x5b
	.byte	0x48
	.byte	0x0
	.align	1
L.314:
	.byte	0x1b
	.byte	0x5b
	.byte	0x43
	.byte	0x0
	.align	1
L.313:
	.byte	0x1b
	.byte	0x5b
	.byte	0x44
	.byte	0x0
	.align	1
L.312:
	.byte	0x1b
	.byte	0x5b
	.byte	0x42
	.byte	0x0
	.align	1
L.311:
	.byte	0x1b
	.byte	0x5b
	.byte	0x41
	.byte	0x0
	.align	1
L.236:
	.byte	0x28
	.byte	0x4e
	.byte	0x55
	.byte	0x4c
	.byte	0x4c
	.byte	0x29
	.byte	0x0
	.align 4
