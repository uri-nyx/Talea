; Headers & drivers for the keyboard device in the Taleä System
#once
#include "headers.asm"


; Subroutines
__kb:
	.modifiers:
	__asm_inline_kbd_get_modifiers
	ret
	.char:
	__asm_inline_kbd_get_char
	ret
	.key:
	__asm_inline_kbd_get_key
	ret