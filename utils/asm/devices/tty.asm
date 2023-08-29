; Headers & drivers for the serial device in the Taleä System
#once
#include "headers.asm"

; Subroutines
__serial:
	.send:
	__asm_inline_tty_send
	ret

	.receive:
	__asm_inline_tty_receive_one_unsigned
	sb a0, 0(a1)
	addi a1, a1, 1
	__asm_inline_tty_input_len
	bne a0, zero, .receive
	ret

	.receive_one:
	__asm_inline_tty_receive_one
	ret

	.input_len:
	__asm_inline_tty_input_len
	ret

	.set:
	
		..fifo:
		li a0, T_mode_FIFO
		__asm_inline_tty_set_mode
		ret

		..lifo:
		li a0, T_mode_LIFO
		__asm_inline_tty_set_mode
		ret

