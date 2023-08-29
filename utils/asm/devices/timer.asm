; Headers & drivers for the timer device in the Taleä System

#once
#include "headers.asm"

; Subroutines
__timer:
	.timeout:

	..set:
	__asm_inline_timer_set_timeout_freq
	__asm_inline_timer_set_timeout
	ret

	..get:
	__asm_inline_timer_get_timeout_freq
	__asm_inline_timer_get_timeout
	ret

	..disable:
	mv a1, zero
	__asm_inline_timer_set_timeout_freq
	ret

	..enable:
	andi a1, a1, 0xff
	seqz t0, a1    
	add a1, a1, t0 ; To prevent that 
				   ; enable stops the timer
	__asm_inline_timer_set_timeout_freq
	ret

	.interval:
		..set:
	__asm_inline_timer_set_interval_freq
	__asm_inline_timer_set_interval
	ret

	..get:
	__asm_inline_timer_get_interval_freq
	__asm_inline_timer_get_interval
	ret

	..disable:
	mv a1, zero
	__asm_inline_timer_set_interval_freq
	ret

	..enable:
	andi a1, a1, 0xff
	seqz t0, a1    
	add a1, a1, t0 ; To prevent that 
				   ; enable stops the timer
	__asm_inline_timer_set_interval_freq
	ret
