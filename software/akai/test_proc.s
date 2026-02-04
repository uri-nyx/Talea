# at 0x25000
_start:
	li x5, 0xb000b
	trace x5,x0,x0,x1
	li x12, 1 	# SYSCALL_YIELD
	syscall x0, 0x40
	trace x1,x0,x0,x5
	li x12, 1 	# SYSCALL_YIELD
	syscall x0, 0x40
	li x12, 0 	# SYSCALL_EXIT
	mv x13, x5
	syscall x0, 0x40
				
