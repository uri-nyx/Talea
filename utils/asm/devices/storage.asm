; Headers & drivers for the storage devices in the Taleä System
#once
#include "headers.asm"

; Subroutines
__disk:
	.load:
	__asm_inline_disk_load
	ret

	.store:
	__asm_inline_disk_store
	ret

	.check:
		..bootable:
		__asm_inline_disk_bootable
		ret

__tps:
	.load:
	__asm_inline_tps_load
	ret

	.store:
	__asm_inline_disk_store
	ret

	.check:
		..bootable:
		__asm_inline_tps_bootable
		ret

	.open:
	__asm_inline_tps_open
	ret

	.close:
	__asm_inline_tps_close
	ret

	.select:
		..a:
		__asm_inline_tps_set_a
		ret

		..b:
		__asm_inline_tps_set_b
		ret


