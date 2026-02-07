# at 0x25000
_start:
	li x12, 4	# SYSCALL_IPC_INIT
	syscall x0, 0x40
	beq x0, x10, continue
	# exit if ipc could not initiate
		li x12, 0 	# SYSCALL_EXIT
		li x13, 0xdead	
		syscall x0, 0x40
continue:
	li x5, 0xb000b
	trace x5,x0,x0,x1
	li x12, 1 	# SYSCALL_YIELD
	syscall x0, 0x40
	trace x1,x0,x0,x5
	li x12, 2 	# SYSCALL_HOOK
	li x13, 0xFFF # ALL EVENTS
	la x14, handle_events
	syscall x0, 0x40
_idle:
	
				li 		x6, 0xfffff
	bussyloop:	addi 	x6, x6, -1
				bgtu	x6, x0, bussyloop
			   

	trace x1,x0,x10,x5
	li x12, 1 	# SYSCALL_YIELD
	syscall x0, 0x40
	j _idle


handle_events:
	trace x31, x31, x31, x2
	push x12, x2
	push x13, x2
	li x13, 0xF81000
	li x12, 5 # SYSCALL PUTC
	lbu x13, 0(x13)
	syscall x0, 0x40
	li x12, 0xF7F000
	li x13, 0
	sb x13, 0(x12)
	pop x13, x2
	pop x12, x2
	jalr x0, 0(x31) # magic!

				
