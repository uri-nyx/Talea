# Compiled with lcc-sirius 4.2
# assemble with supplied as assembler
# link with supplied ld linker
	.align 4
	.text
	.globl test_ll_constants
	.align	4
test_ll_constants:
	addi x2,x2,-96
	sw  x8,92(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	li x28, 0x112233
	li x28^, 0x44556677

	sw x28, +(-16)+48(x8)
	sw x28^, 4+(-16)+48(x8)
	li x28, 0xffeeddcc
	li x28^, 0xbbaa9988

	sw x28, +(-24)+48(x8)
	sw x28^, 4+(-24)+48(x8)
	li x28, 0x0
	li x28^, 0x0
	sw x28, +(-32)+48(x8)
	sw x28^, 4+(-32)+48(x8)
	li x28, 0x0
	li x28^, 0x1

	sw x28, +(-40)+48(x8)
	sw x28^, 4+(-40)+48(x8)
	li x28, 0xffffffff
	li x28^, 0xffffffff

	sw x28, +(-48)+48(x8)
	sw x28^, 4+(-48)+48(x8)
	lw x12,+(-16)+48(x8)
	lw x12^, 4+(-16)+48(x8)
	lw x14,+(-24)+48(x8)
	lw x14^, 4+(-24)+48(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	lw x14,+(-32)+48(x8)
	lw x14^, 4+(-32)+48(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	lw x14,+(-40)+48(x8)
	lw x14^, 4+(-40)+48(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	lw x14,+(-48)+48(x8)
	lw x14^, 4+(-48)+48(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.1:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,92(x2)
	addi  x2,x2,96
	jalr x0,0(x1)

	.globl test_ll_arithmetic
	.align	4
test_ll_arithmetic:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,64
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x25,36(x2)
	sw x26,40(x2)
	sw x27,44(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	sw x15,28(x8)
	lw x12,+(0)+16(x8)
	lw x12^, 4+(0)+16(x8)
	lw x14,+(8)+16(x8)
	lw x14^, 4+(8)+16(x8)
	call __add64
	mv x26,x10
	mv x26^,x10^ ; LOADI8
	lw x12,+(0)+16(x8)
	lw x12^, 4+(0)+16(x8)
	lw x14,+(8)+16(x8)
	lw x14^, 4+(8)+16(x8)
	call __sub64
	mv x24,x10
	mv x24^,x10^ ; LOADI8
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	mv x14,x24
	mv x14^,x24^ ; LOADI8
	call __muli64
	sw x10, +(-16)+16(x8)
	sw x10^, 4+(-16)+16(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	mv x14,x24
	mv x14^,x24^ ; LOADI8
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.2:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x25,36(x2)
	lw x26,40(x2)
	lw x27,44(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl test_ll_bitwise
	.align	4
test_ll_bitwise:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x27,40(x2)
	mv x26,x12
	sw x14,40(x8)
	sw x15,44(x8)
	lw x28,+(8)+32(x8)
	lw x28^, 4+(8)+32(x8)
	sw x28, +(-16)+32(x8)
	sw x28^, 4+(-16)+32(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	mv x14,x28
	mv x14^,x28^ ; LOADI8
	call __and64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	sw x28, +(-24)+32(x8)
	sw x28^, 4+(-24)+32(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	lw x6,+(-16)+32(x8)
	lw x6^, 4+(-16)+32(x8)
	mv x14,x6
	mv x14^,x6^ ; LOADI8
	call __xor64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	lw x6,+(-24)+32(x8)
	lw x6^, 4+(-24)+32(x8)
	mv x12,x6
	mv x12^,x6^ ; LOADI8
	mv x14,x28
	mv x14^,x28^ ; LOADI8
	call __or64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	sw x28, +(-32)+32(x8)
	sw x28^, 4+(-32)+32(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	call __not64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	lw x6,+(-32)+32(x8)
	lw x6^, 4+(-32)+32(x8)
	mv x12,x6
	mv x12^,x6^ ; LOADI8
	mv x14,x28
	mv x14^,x28^ ; LOADI8
	call __or64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.3:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw x27,40(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl test_ll_shifts
	.align	4
test_ll_shifts:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x27,40(x2)
	mv x26,x12
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	li x14,4
	call __shll64
	sw x10, +(-16)+32(x8)
	sw x10^, 4+(-16)+32(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADI8
	li x14,4
	call __shra64
	sw x10, +(-24)+32(x8)
	sw x10^, 4+(-24)+32(x8)
	mv x12,x26
	mv x12^,x26^ ; LOADU8
	li x14,4
	call __shrl64
	sw x10, +(-32)+32(x8)
	sw x10^, 4+(-32)+32(x8)
	lw x12,+(-16)+32(x8)
	lw x12^, 4+(-16)+32(x8)
	lw x14,+(-24)+32(x8)
	lw x14^, 4+(-24)+32(x8)
	call __add64
	mv x12,x10
	mv x12^,x10^ ; LOADI8
	lw x28,+(-32)+32(x8)
	lw x28^, 4+(-32)+32(x8)
	mv x14,x28
	mv x14^,x28^ ; LOADI8
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.4:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw x27,40(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl test_ll_compare_signed
	.align	4
test_ll_compare_signed:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x22,0(x2)
	sw x24,4(x2)
	sw x26,8(x2)
	sw x27,12(x2)
	mv x27,x0 ; LOADI4
;NEI8s
	xor x5, x12^, x14^
	bne x5, x0, L.6
	xor x5, x12, x14
	bne x5, x0, L.6
	ori x27,x27,1
L.6:
;EQI8s
	xor x5, x12^, x14^
	bne x5, x0, 12
	xor x5, x12, x14
	beq x5, x0, L.8
	ori x27,x27,2
L.8:
;GEI8s
	bgt x12, x14, L.10
	blt x12, x14, 8
	bgeu x12^, x14^, L.10
	ori x27,x27,4
L.10:
;GTI8s
	bgt x12, x14, L.12
	bne x12, x14, 8
	bgtu x12^, x14^, L.12
	ori x27,x27,8
L.12:
;LEI8s
	blt x12, x14, L.14
	bgt x12, x14, 8
	bleu x12^, x14^, L.14
	ori x27,x27,16
L.14:
;LTI8s
	blt x12, x14, L.16
	bne x12, x14, 8
	bltu x12^, x14^, L.16
	ori x27,x27,32
L.16:
	mv x10,x27 ; LOADI4
L.5:
	lw x22,0(x2)
	lw x24,4(x2)
	lw x26,8(x2)
	lw x27,12(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_ll_compare_unsigned
	.align	4
test_ll_compare_unsigned:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x22,0(x2)
	sw x24,4(x2)
	sw x26,8(x2)
	sw x27,12(x2)
	mv x27,x0 ; LOADI4
;NEU8s
	xor x5, x12^, x14^
	bne x5, x0, L.19
	xor x5, x12, x14
	bne x5, x0, L.19
	ori x27,x27,1
L.19:
;EQU8s
	xor x5, x12^, x14^
	bne x5, x0, 12
	xor x5, x12, x14
	beq x5, x0, L.21
	ori x27,x27,2
L.21:
;GEU8s
	bgtu x12, x14, L.23
	bltu x12, x14, 8
	bgeu x12^, x14^, L.23
	ori x27,x27,4
L.23:
;GTU8s
	bgtu x12, x14, L.25
	bne x12, x14, 8
	bgtu x12^, x14^, L.25
	ori x27,x27,8
L.25:
;LEU8s
	bltu x12, x14, L.27
	bgtu x12, x14, 8
	bleu x12^, x14^, L.27
	ori x27,x27,16
L.27:
;LTU8s
	bltu x12, x14, L.29
	bne x12, x14, 8
	bltu x12^, x14^, L.29
	ori x27,x27,32
L.29:
	mv x10,x27 ; LOADI4
L.18:
	lw x22,0(x2)
	lw x24,4(x2)
	lw x26,8(x2)
	lw x27,12(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_conversions_i32_to_i64
	.align	4
test_conversions_i32_to_i64:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
;CVII8
	mv x10^, x12
	shrai x10, x12, 31
L.31:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_conversions_u32_to_u64
	.align	4
test_conversions_u32_to_u64:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
;CVUI8
	mv x10^, x12
	mv x10, x0
L.32:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_conversions_i64_to_i32
	.align	4
test_conversions_i64_to_i32:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	mv x10,x12^ ; LOADI4 from pair (trunc 64->32)
L.33:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_conversions_u64_to_u32
	.align	4
test_conversions_u64_to_u32:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	mv x10,x12^ ; LOADU4 from pair (trunc 64->32)
L.34:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i32_arithmetic
	.align	4
test_i32_arithmetic:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	add x30,x12,x13
	sub x29,x12,x13
	imul x0,x10,x30,x29
L.35:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i32_bitwise
	.align	4
test_i32_bitwise:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	and x30,x12,x13
	xor x29,x12,x13
	or x30,x30,x29
	not x29,x12
	or x10,x30,x29
L.36:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i32_shifts
	.align	4
test_i32_shifts:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,3
	shll x29,x12,x30
	shra x28,x12,x30
	add x29,x29,x28
	mv x28,x12 ; LOADU4
	shrl x30,x28,x30
	add x30,x29,x30
	mv x10,x30 ; LOADI4
L.37:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i32_compare
	.align	4
test_i32_compare:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	mv x27,x0 ; LOADI4
	bne x12,x13,L.39
	ori x27,x27,1
L.39:
	beq x12,x13,L.41
	ori x27,x27,2
L.41:
	bge x12,x13,L.43
	ori x27,x27,4
L.43:
	ble x12,x13,L.45
	ori x27,x27,8
L.45:
	mv x10,x27 ; LOADI4
L.38:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i16_ops
	.align	4
test_i16_ops:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x12,8*(4-2)
	shrai x30,x30,8*(4-2)
	shlli x29,x13,8*(4-2)
	shrai x29,x29,8*(4-2)
	add x28,x30,x29
	imul x0,x30,x30,x29
	sub x30,x28,x30
	shlli x10,x30,8*(4-2)
	shrai x10,x10,8*(4-2)
L.47:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_i8_ops
	.align	4
test_i8_ops:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	shlli x29,x13,8*(4-1)
	shrli x29,x29,8*(4-1)
	add x28,x30,x29
	or x30,x30,x29
	and x30,x28,x30
	shlli x10,x30,8*(4-1)
	shrli x10,x10,8*(4-1)
L.48:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_u16_ops
	.align	4
test_u16_ops:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x12,8*(4-2)
	shrli x30,x30,8*(4-2)
	shlli x29,x13,8*(4-2)
	shrli x29,x29,8*(4-2)
	add x28,x30,x29
	sub x30,x30,x29
	xor x30,x28,x30
	shlli x10,x30,8*(4-2)
	shrli x10,x10,8*(4-2)
L.49:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_u8_ops
	.align	4
test_u8_ops:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	shlli x30,x12,8*(4-1)
	shrli x30,x30,8*(4-1)
	li x29,1
	shll x30,x30,x29
	shlli x29,x13,8*(4-1)
	shrli x29,x29,8*(4-1)
	li x28,2
	idiv x29,x0,x29,x28
	add x30,x30,x29
	shlli x10,x30,8*(4-1)
	shrli x10,x10,8*(4-1)
L.50:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_pointer_arithmetic
	.align	4
test_pointer_arithmetic:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x27,0(x2)
	li x30,2
	shll x30,x13,x30
	add x27,x30,x12
	lw x30,+0(x27)
	li x29,4
	add x29,x27,x29
	lw x29,+0(x29)
	add x10,x30,x29
L.51:
	lw x27,0(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_ll_pointer
	.align	4
test_ll_pointer:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x27,40(x2)
	mv x27,x12
	lw x28,+0(x27)
	lw x28^, 4+0(x27)
	sw x28, +(-16)+16(x8)
	sw x28^, 4+(-16)+16(x8)
	li x30,8
	add x27,x27,x30
	lw x12,+(-16)+16(x8)
	lw x12^, 4+(-16)+16(x8)
	li x14, 0x0
	li x14^, 0x1

	call __add64
	sw x10, +0(x27)
	sw x10^, 4+0(x27)
	lw x10,+0(x27)
	lw x10^, 4+0(x27)
L.52:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw x27,40(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_pointer_store
	.align	4
test_pointer_store:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x13,+0(x12)
	li x30,4
	add x30,x12,x30
	addi x29,x13,1
	sw x29,+0(x30)
	li x30,8
	add x30,x12,x30
	addi x29,x13,2
	sw x29,+0(x30)
L.53:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.55:
	.word	0xa
	.word	0x14
	.word	0x1e
	.word	0x28
	.word	0x32
	.globl test_array_i32
	.align 4
	.text
	.align	4
test_array_i32:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	sw x26,0(x2)
	sw x27,4(x2)
	addi x30,x8,-24+32
	la x5,L.55
	li x28,20
	copy x5,x30,x28
	mv x26,x0 ; LOADI4
	mv x27,x0 ; LOADI4
L.56:
	li x30,2
	shll x30,x27,x30
	addi x29,x8,-24+32
	add x30,x30,x29
	lw x30,+0(x30)
	add x26,x26,x30
L.57:
	addi x27,x27,1
	li x30,5
	blt x27,x30,L.56
	mv x10,x26 ; LOADI4
L.54:
	lw x26,0(x2)
	lw x27,4(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.align	8
L.61:
	.word	0x11111111
	.word	0x11111111
	.word	0x22222222
	.word	0x22222222
	.word	0x33333333
	.word	0x33333333
	.globl test_array_i64
	.align 4
	.text
	.align	4
test_array_i64:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	addi x30,x8,-32+32
	la x5,L.61
	li x28,24
	copy x5,x30,x28
	lw x12,+(-32)+32(x8)
	lw x12^, 4+(-32)+32(x8)
	lw x14,+(-24)+32(x8)
	lw x14^, 4+(-24)+32(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	lw x14,+(-16)+32(x8)
	lw x14^, 4+(-16)+32(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.60:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.globl test_struct_ref
	.align	4
test_struct_ref:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x30,+0(x12)
	li x29,4
	add x29,x12,x29
	lw x29,+0(x29)
	add x10,x30,x29
L.64:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_modify
	.align	4
test_struct_modify:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,100
	sw x30,+0(x12)
	li x30,4
	add x30,x12,x30
	li x29,200
	sw x29,+0(x30)
L.65:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_return_ref
	.align	4
test_struct_return_ref:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x11, 16(x8)
	li x30,42
	sw x30,+(-12)+16(x8)
	li x30,84
	sw x30,+(-8)+16(x8)
	lw x30,+(0)+16(x8)
	addi x5,x8,-12+16
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
L.66:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_small_byval
	.align	4
test_struct_small_byval:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x30,+0(x12)
	li x29,4
	add x29,x12,x29
	lw x29,+0(x29)
	add x10,x30,x29
L.69:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_small_return
	.align	4
test_struct_small_return:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x11, 16(x8)
	sw x12,+(-12)+16(x8)
	sw x13,+(-8)+16(x8)
	lw x30,+(0)+16(x8)
	addi x5,x8,-12+16
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
L.70:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_medium_byval
	.align	4
test_struct_medium_byval:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x30,+0(x12)
	li x29,4
	add x29,x12,x29
	lw x29,+0(x29)
	add x30,x30,x29
	li x29,8
	add x29,x12,x29
	lw x29,+0(x29)
	add x30,x30,x29
	li x29,12
	add x29,x12,x29
	lw x29,+0(x29)
	add x10,x30,x29
L.73:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_medium_return
	.align	4
test_struct_medium_return:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	sw x11, 32(x8)
	li x30,1
	sw x30,+(-20)+32(x8)
	li x30,2
	sw x30,+(-16)+32(x8)
	li x30,3
	sw x30,+(-12)+32(x8)
	li x30,4
	sw x30,+(-8)+32(x8)
	lw x30,+(0)+32(x8)
	addi x5,x8,-20+32
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
L.74:
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_struct_large_byval
	.align	4
test_struct_large_byval:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x12,16(x8)
	lw x30,+(0)+16(x8)
	lw x12,+0(x30)
	lw x12^, 4+0(x30)
	li x29,8
	add x29,x30,x29
	lw x14,+0(x29)
	lw x14^, 4+0(x29)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	li x29,16
	add x30,x30,x29
	lw x30,+0(x30)
;CVII8
	mv x14^, x30
	shrai x14, x30, 31
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.79:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_struct_large_return
	.align	4
test_struct_large_return:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	sw x11, 32(x8)
	li x28, 0x11111111
	li x28^, 0x11111111

	sw x28, +(-32)+32(x8)
	sw x28^, 4+(-32)+32(x8)
	li x28, 0x22222222
	li x28^, 0x22222222

	sw x28, +(-24)+32(x8)
	sw x28^, 4+(-24)+32(x8)
	li x30,333
	sw x30,+(-16)+32(x8)
	lw x30,+(0)+32(x8)
	addi x5,x8,-32+32
	li x28,24
	copy x5,x30,x28
L.80:
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_struct_nested_byval
	.align	4
test_struct_nested_byval:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x30,+0(x12)
	li x29,4
	add x29,x12,x29
	lw x29,+0(x29)
	add x30,x30,x29
	li x29,8
	add x29,x12,x29
	lw x29,+0(x29)
	add x10,x30,x29
L.84:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_with_ll
	.align	4
test_struct_with_ll:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x12,16(x8)
	lw x30,+(0)+16(x8)
	lw x12,+0(x30)
	lw x12^, 4+0(x30)
	li x29,8
	add x30,x30,x29
	lw x30,+0(x30)
;CVII8
	mv x14^, x30
	shrai x14, x30, 31
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.85:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_return_struct_with_ll
	.align	4
test_return_struct_with_ll:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	sw x11, 32(x8)
	sw x12, +(-24)+32(x8)
	sw x12^, 4+(-24)+32(x8)
	li x30,42
	sw x30,+(-16)+32(x8)
	lw x30,+(0)+32(x8)
	addi x5,x8,-24+32
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
L.86:
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_float_arithmetic
	.align	4
test_float_arithmetic:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x30,+(0)+16(x8)
	sw x30,+(-8)+16(x8)
	lw x29,+(4)+16(x8)
	sw x29,+(-12)+16(x8)
	addi x12,x30,0
	addi x13,x29,0
	call float32_add
	addi x30,x10,0
	sw x30,+(-16)+16(x8)
	lw x29,+(-8)+16(x8)
	addi x12,x29,0
	lw x29,+(-12)+16(x8)
	addi x13,x29,0
	call float32_sub
	addi x30,x10,0
	lw x29,+(-16)+16(x8)
	addi x12,x29,0
	addi x13,x30,0
	call float32_mul
	addi x30,x10,0
	addi x10,x30,0
L.89:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_float_compare
	.align	4
test_float_compare:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x26,0(x2)
	sw x27,4(x2)
	mv x27,x12
	mv x26,x13
	addi x12,x27,0
	addi x13,x26,0
	call float32_ge
	bne x10,x0,L.91
	la x30,L.93
	lw x10,+0(x30)
	j L.90
L.91:
	addi x12,x27,0
	addi x13,x26,0
	call float32_le
	bne x10,x0,L.94
	la x30,L.96
	lw x10,+0(x30)
	j L.90
L.94:
	addi x12,x27,0
	addi x13,x26,0
	call float32_ne
	bne x10,x0,L.97
	la x30,L.99
	lw x10,+0(x30)
	j L.90
L.97:
	la x30,L.100
	lw x10,+0(x30)
L.90:
	lw x26,0(x2)
	lw x27,4(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_float_conversion
	.align	4
test_float_conversion:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x12,+(0)+16(x8)
	call int32_to_float32
	addi x30,x10,0
	addi x10,x30,0
L.101:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_float_to_int
	.align	4
test_float_to_int:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	lw x12,+(0)+16(x8)
	call float32_to_int32
	mv x30,x10 ; LOADI4
L.102:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_double_arithmetic
	.align	4
test_double_arithmetic:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,32(x8)
	sw x13,36(x8)
	sw x14,40(x8)
	sw x15,44(x8)
	lw x28,+(0)+32(x8)
	lw x28^, 4+(0)+32(x8)
	sw x28, +(-16)+32(x8)
	sw x28^, 4+(-16)+32(x8)
	lw x6,+(8)+32(x8)
	lw x6^, 4+(8)+32(x8)
	sw x6, +(-24)+32(x8)
	sw x6^, 4+(-24)+32(x8)
	mv x12,x28
	mv x12^,x28^
	mv x14,x6
	mv x14^,x6^
	call double_mul
	mv x28,x10
	mv x28^,x10^
	sw x28, +(-32)+32(x8)
	sw x28^, 4+(-32)+32(x8)
	lw x6,+(-16)+32(x8)
	lw x6^, 4+(-16)+32(x8)
	mv x12,x6
	mv x12^,x6^
	lw x6,+(-24)+32(x8)
	lw x6^, 4+(-24)+32(x8)
	mv x14,x6
	mv x14^,x6^
	call double_div
	mv x28,x10
	mv x28^,x10^
	lw x6,+(-32)+32(x8)
	lw x6^, 4+(-32)+32(x8)
	mv x12,x6
	mv x12^,x6^
	mv x14,x28
	mv x14^,x28^
	call double_add
	mv x28,x10
	mv x28^,x10^
L.103:
	lw x1,24(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_double_conversion
	.align	4
test_double_conversion:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x12,+(0)+16(x8)
	lw x12^, 4+(0)+16(x8)
	call int64_to_double
	mv x28,x10
	mv x28^,x10^
L.104:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_double_to_ll
	.align	4
test_double_to_ll:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	lw x12,+(0)+16(x8)
	lw x12^, 4+(0)+16(x8)
	call double_to_int64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.105:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.align 4
	.data
	.globl global_i32
	.align	4
global_i32:
	.word	0x3039
	.globl global_i64
	.align	8
global_i64:
	.word	0x1234567
	.word	0x89abcdef
	.globl global_struct
	.align	4
global_struct:
	.word	0x64
	.word	0xc8
	.globl test_global_i32
	.align 4
	.text
	.align	4
test_global_i32:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	la x30,global_i32
	lw x29,+0(x30)
	addi x29,x29,10
	sw x29,+0(x30)
	lw x10,+0(x30)
L.106:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_global_i64
	.align	4
test_global_i64:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	la x30,global_i64
	lw x28,+0(x30)
	lw x28^, 4+0(x30)
	mv x12,x28
	mv x12^,x28^ ; LOADU8
	li x14, 0x0
	li x14^, 0x1111

	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	sw x28, +0(x30)
	sw x28^, 4+0(x30)
	lw x10,+0(x30)
	lw x10^, 4+0(x30)
L.107:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_global_struct
	.align	4
test_global_struct:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	la x30,global_struct
	lw x29,+0(x30)
	li x28,1
	shll x29,x29,x28
	sw x29,+0(x30)
	la x30,global_struct+4
	li x29,3
	lw x28,+0(x30)
	imul x0,x29,x29,x28
	sw x29,+0(x30)
	la x30,global_struct
	lw x30,+0(x30)
	la x29,global_struct+4
	lw x29,+0(x29)
	add x10,x30,x29
L.108:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_if_else
	.align	4
test_if_else:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	bge x12,x0,L.112
	sub x10,x0,x12
	j L.111
L.112:
	li x30,100
	ble x12,x30,L.114
	li x10,100
	j L.111
L.114:
	mv x10,x12 ; LOADI4
L.111:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_switch
	.align	4
test_switch:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	beq x12,x0,L.119
	li x30,1
	beq x12,x30,L.120
	li x30,2
	beq x12,x30,L.121
	j L.117
L.119:
	li x10,100
	j L.116
L.120:
	li x10,200
	j L.116
L.121:
	li x10,300
	j L.116
L.117:
	li x10,-1
L.116:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_while_loop
	.align	4
test_while_loop:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x26,0(x2)
	sw x27,4(x2)
	mv x26,x0 ; LOADI4
	mv x27,x0 ; LOADI4
	j L.124
L.123:
	add x26,x26,x27
	addi x27,x27,1
L.124:
	blt x27,x12,L.123
	mv x10,x26 ; LOADI4
L.122:
	lw x26,0(x2)
	lw x27,4(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_for_loop
	.align	4
test_for_loop:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x26,0(x2)
	sw x27,4(x2)
	mv x26,x0 ; LOADI4
	mv x27,x0 ; LOADI4
	j L.130
L.127:
	imul x0,x30,x27,x27
	add x26,x26,x30
L.128:
	addi x27,x27,1
L.130:
	blt x27,x12,L.127
	mv x10,x26 ; LOADI4
L.126:
	lw x26,0(x2)
	lw x27,4(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_do_while
	.align	4
test_do_while:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	sw x26,0(x2)
	sw x27,4(x2)
	mv x26,x0 ; LOADI4
	mv x27,x0 ; LOADI4
L.132:
	add x26,x26,x27
	addi x27,x27,1
L.133:
	blt x27,x12,L.132
	mv x10,x26 ; LOADI4
L.131:
	lw x26,0(x2)
	lw x27,4(x2)
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl helper_add
	.align	4
helper_add:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	add x10,x12,x13
L.135:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_simple_call
	.align	4
test_simple_call:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	li x12,10
	li x13,20
	call helper_add
	mv x30,x10 ; LOADI4
L.136:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl helper_ll_add
	.align	4
helper_ll_add:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	sw x12,16(x8)
	sw x13,20(x8)
	sw x14,24(x8)
	sw x15,28(x8)
	sw x16,32(x8)
	sw x17,36(x8)
	lw x12,+(0)+16(x8)
	lw x12^, 4+(0)+16(x8)
	lw x14,+(8)+16(x8)
	lw x14^, 4+(8)+16(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x12,x28
	mv x12^,x28^ ; LOADI8
	lw x14,+(16)+16(x8)
	lw x14^, 4+(16)+16(x8)
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.137:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_ll_call
	.align	4
test_ll_call:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	li x12, 0x11111111
	li x12^, 0x11111111

	li x14, 0x22222222
	li x14^, 0x22222222

	li x16, 0x33333333
	li x16^, 0x33333333

	call helper_ll_add
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.138:
	lw x1,24(x2)
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl helper_struct_extract
	.align	4
helper_struct_extract:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x30,+0(x12)
	li x29,4
	add x29,x12,x29
	lw x29,+0(x29)
	imul x0,x10,x30,x29
L.139:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_struct_call
	.align	4
test_struct_call:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,32
	sw x1,24(x2)
	li x30,5
	sw x30,+(-12)+32(x8)
	li x30,7
	sw x30,+(-8)+32(x8)
	addi x30,x8,-20+32
	addi x5,x8,-12+32
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	addi x12,x8,-20+32
	call helper_struct_extract
	mv x30,x10 ; LOADI4
L.140:
	lw x1,24(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_overflow_add
	.align	4
test_overflow_add:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	li x28, 0x7fffffff
	li x28^, 0xffffffff

	sw x28, +(-16)+16(x8)
	sw x28^, 4+(-16)+16(x8)
	lw x12,+(-16)+16(x8)
	lw x12^, 4+(-16)+16(x8)
	li x14, 0x0
	li x14^, 0x1

	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.142:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_underflow_sub
	.align	4
test_underflow_sub:
	addi x2,x2,-64
	sw  x8,60(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	li x28, 0x80000000
	li x28^, 0x0

	sw x28, +(-16)+16(x8)
	sw x28^, 4+(-16)+16(x8)
	lw x12,+(-16)+16(x8)
	lw x12^, 4+(-16)+16(x8)
	li x14, 0x0
	li x14^, 0x1

	call __sub64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.143:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,60(x2)
	addi  x2,x2,64
	jalr x0,0(x1)

	.globl test_divide_by_zero_safe
	.align	4
test_divide_by_zero_safe:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	bne x13,x0,L.145
	mv x10,x0 ; LOADI4
	j L.144
L.145:
	idiv x10,x0,x12,x13
L.144:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_modulo
	.align	4
test_modulo:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x30,10
	idiv x0,x30,x12,x30
	li x29,7
	idiv x0,x29,x12,x29
	add x30,x30,x29
	li x29,3
	idiv x0,x29,x12,x29
	add x10,x30,x29
L.147:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_bit_patterns
	.align	4
test_bit_patterns:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	li x28, 0xffffffff
	li x28^, 0xffffffff

	sw x28, +(-16)+32(x8)
	sw x28^, 4+(-16)+32(x8)
	li x28, 0xaaaaaaaa
	li x28^, 0xaaaaaaaa

	sw x28, +(-24)+32(x8)
	sw x28^, 4+(-24)+32(x8)
	li x28, 0x55555555
	li x28^, 0x55555555

	sw x28, +(-32)+32(x8)
	sw x28^, 4+(-32)+32(x8)
	lw x12,+(-24)+32(x8)
	lw x12^, 4+(-24)+32(x8)
	lw x14,+(-32)+32(x8)
	lw x14^, 4+(-32)+32(x8)
	call __xor64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	lw x12,+(-16)+32(x8)
	lw x12^, 4+(-16)+32(x8)
	mv x14,x28
	mv x14^,x28^ ; LOADI8
	call __add64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
L.148:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.150:
	.word	0xa
	.word	0x14
	.align	4
L.151:
	.word	0x5
	.word	0xa
	.align	4
L.152:
	.word	0x1
	.word	0x2
	.word	0x3
	.word	0x4
	.align	8
L.153:
	.word	0x1
	.word	0x23456789
	.word	0x64
	.space	4
	.globl main
	.align 4
	.text
	.align	4
main:
	addi x2,x2,-208
	sw  x8,204(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x27,28(x2)
	mv x27,x0 ; LOADI4
	addi x30,x8,-12+160
	la x5,L.150
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	addi x30,x8,-56+160
	la x5,L.151
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	li x12,15
	li x13,25
	addi x11,x8,-48+160
	call test_struct_small_return
	addi x30,x8,-72+160
	la x5,L.152
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
	addi x30,x8,-88+160
	la x5,L.153
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
	call test_ll_constants
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x12, 0x0
	li x12^, 0x3e8

	li x14, 0x0
	li x14^, 0x1f4

	call test_ll_arithmetic
	mv x30,x10^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x28, 0xff00ff00
	li x28^, 0xff00ff00

	mv x12,x28
	mv x12^,x28^ ; LOADI8
	li x14, 0xff00ff
	li x14^, 0xff00ff

	call test_ll_bitwise
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x12, 0x0
	li x12^, 0x64

	li x14, 0x0
	li x14^, 0x32

	call test_ll_compare_signed
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12, 0x0
	li x12^, 0x64

	li x14, 0x0
	li x14^, 0x32

	call test_ll_compare_unsigned
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,-1000
	call test_conversions_i32_to_i64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x12,2000
	call test_conversions_u32_to_u64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x12, 0x1234567
	li x12^, 0x89abcdef

	call test_conversions_i64_to_i32
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,100
	li x13,50
	call test_i32_arithmetic
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,43690
	li x13,21845
	call test_i32_bitwise
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,256
	call test_i32_shifts
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,10
	li x13,20
	call test_i32_compare
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,100
	li x13,50
	call test_i16_ops
	mv x30,x10 ; LOADI2
	shlli x30,x30,8*(4-2)
	shrai x30,x30,8*(4-2)
	add x27,x27,x30
	li x12,10
	li x13,5
	call test_i8_ops
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	add x27,x27,x30
	li x12,200
	li x13,100
	call test_u16_ops
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-2)
	shrli x30,x30,8*(4-2)
	add x27,x27,x30
	li x12,50
	li x13,10
	call test_u8_ops
	mv x30,x10 ; LOADU4
	shlli x30,x30,8*(4-1)
	shrli x30,x30,8*(4-1)
	add x27,x27,x30
	call test_array_i32
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_array_i64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	addi x12,x8,-12+160
	call test_struct_ref
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	addi x12,x8,-12+160
	call test_struct_modify
	lw x30,+(-12)+160(x8)
	lw x29,+(-8)+160(x8)
	add x30,x30,x29
	add x27,x27,x30
	addi x30,x8,-96+160
	addi x5,x8,-56+160
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	addi x12,x8,-96+160
	call test_struct_small_byval
	add x27,x27,x10
	lw x30,+(-48)+160(x8)
	lw x29,+(-44)+160(x8)
	add x30,x30,x29
	add x27,x27,x30
	addi x30,x8,-112+160
	addi x5,x8,-72+160
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
	addi x12,x8,-112+160
	call test_struct_medium_byval
	add x27,x27,x10
	li x28, 0x0
	li x28^, 0x3e8

	sw x28, +(-40)+160(x8)
	sw x28^, 4+(-40)+160(x8)
	li x28, 0x0
	li x28^, 0x7d0

	sw x28, +(-32)+160(x8)
	sw x28^, 4+(-32)+160(x8)
	li x30,30
	sw x30,+(-24)+160(x8)
	addi x30,x8,-136+160
	addi x5,x8,-40+160
	li x28,24
	copy x5,x30,x28
	addi x12,x8,-136+160
	call test_struct_large_byval
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	addi x30,x8,-152+160
	addi x5,x8,-88+160
	lw x28,0(x5)
	lw x6,4(x5)
	sw	x28, 0(x30)
	sw	x6, 4(x30)
	lw x28,8(x5)
	lw x6,12(x5)
	sw	x28, 8(x30)
	sw	x6, 12(x30)
	addi x12,x8,-152+160
	call test_struct_with_ll
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	call test_global_i32
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_global_i64
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	call test_global_struct
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,50
	call test_if_else
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,1
	call test_switch
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,10
	call test_while_loop
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,10
	call test_for_loop
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,10
	call test_do_while
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_simple_call
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_ll_call
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	call test_struct_call
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_overflow_add
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	li x12,100
	li x13,5
	call test_divide_by_zero_safe
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	li x12,123
	call test_modulo
	mv x30,x10 ; LOADI4
	add x27,x27,x30
	call test_bit_patterns
	mv x28,x10
	mv x28^,x10^ ; LOADI8
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	add x27,x27,x30
	mv x10,x27 ; LOADI4
L.149:
	lw x1,24(x2)
	lw x27,28(x2)
	lw  x8,204(x2)
	addi  x2,x2,208
	jalr x0,0(x1)

	.globl test_endianness_store
	.align	4
test_endianness_store:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	li x28, 0x112233
	li x28^, 0x44556677

	sw x28, +(-16)+16(x8)
	sw x28^, 4+(-16)+16(x8)
	lw x28,+(-16)+16(x8)
	lw x28^, 4+(-16)+16(x8)
	sw x28, +0(x12)
	sw x28^, 4+0(x12)
L.158:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_endianness_load
	.align	4
test_endianness_load:
	addi x2,x2,-32
	sw  x8,28(x2)
	addi  x8,x2,16
	lw x10,+0(x12)
	lw x10^, 4+0(x12)
L.159:
	lw  x8,28(x2)
	addi  x2,x2,32
	jalr x0,0(x1)

	.globl test_endianness_truncate
	.align	4
test_endianness_truncate:
	addi x2,x2,-48
	sw  x8,44(x2)
	addi  x8,x2,16
	li x28, 0x112233
	li x28^, 0x44556677

	sw x28, +(-16)+32(x8)
	sw x28^, 4+(-16)+32(x8)
	lw x28,+(-16)+32(x8)
	lw x28^, 4+(-16)+32(x8)
	mv x30,x28^ ; LOADI4 from pair (trunc 64->32)
	sw x30,+(-20)+32(x8)
	lw x10,+(-20)+32(x8)
L.160:
	lw  x8,44(x2)
	addi  x2,x2,48
	jalr x0,0(x1)

	.globl test_endianness_extend
	.align	4
test_endianness_extend:
	addi x2,x2,-80
	sw  x8,76(x2)
	addi  x8,x2,48
	sw x1,24(x2)
	sw x22,28(x2)
	sw x24,32(x2)
	sw x26,36(x2)
	li x30,0x80000000
	sw x30,+(-8)+32(x8)
	lw x30,+(-8)+32(x8)
;CVII8
	mv x28^, x30
	shrai x28, x30, 31
	sw x28, +(-16)+32(x8)
	sw x28^, 4+(-16)+32(x8)
	lw x12,+(-16)+32(x8)
	lw x12^, 4+(-16)+32(x8)
	li x14,32
	call __shra64
	mv x30,x10^ ; LOADI4 from pair (trunc 64->32)
	sw x30,+(-20)+32(x8)
	lw x10,+(-20)+32(x8)
L.161:
	lw x1,24(x2)
	lw x22,28(x2)
	lw x24,32(x2)
	lw x26,36(x2)
	lw  x8,76(x2)
	addi  x2,x2,80
	jalr x0,0(x1)

	.align 4
	.data
	.align	4
L.100:
	.word	0x0
	.align	4
L.99:
	.word	0x40400000
	.align	4
L.96:
	.word	0x40000000
	.align	4
L.93:
	.word	0x3f800000
	.align 4
