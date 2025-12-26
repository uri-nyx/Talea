#------------------------------------------------------------------------------
# Utility hardware interface routines in Assembly
#------------------------------------------------------------------------------

.globl hw_cli
hw_cli:
    cli
    ret

.globl hw_sti
hw_sti:
    sti
    ret

.globl hw_set_priority
hw_set_priority:
    gsreg x5
    li x6,  0xe3ffffff
    and x5, x5, x6
    shlli x12, x12, 26
    or x5, x5, x12
    ssreg x5 
    ret

.globl hw_syscall
hw_syscall:
    syscall x0, 0x20
    ret

.globl hw_set_x11
hw_set_x11:
    mv x11, x12
    ret

.globl hw_get_x11
hw_get_x11:
    mv x10, x11
    ret

.globl hw_copy
hw_copy:
	copy x12, x13, x14
	ret

.globl hw_fill
hw_fill:
	fill x12, x13, x14
	ret

.globl hw_lhud
hw_lhud:
	lhud x10, 0(x12)
	ret

.globl hw_lwd
hw_lwd:
	lwd x10, 0(x12)
	ret

.globl hw_lhd
hw_lhd:
	lhd x10, 0(x12)
	ret


.globl hw_lbud
hw_lbud:
	lbud x10, 0(x12)
	ret

.globl hw_shd
hw_shd:
	shd x13, 0(x12)
	ret

.globl hw_swd
hw_swd:
	swd x13, 0(x12)
	ret

.globl hw_sbd
hw_sbd:
	sbd x13, 0(x12)
	ret
