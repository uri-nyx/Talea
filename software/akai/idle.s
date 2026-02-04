# PIC
_start:
	trace x0,x0,x0,x2
	li x12, 1 	# SYSCALL_YIELD
	syscall x0, 0x40
	j _start
