#bankdef text {
    #addr 0
    #addr_end 10000
    #outp 8 * 0
}

li  a0, 0b1_1_0_010_111110_11111111_000000000000
ssreg a0                ; supervisor, intterupt enabled, mmu disabled
                        ; priority 2, ivt at 0xf800, pdt at 0xff00
jal ra, INITIALIZE_IVT

li s2, 10000
li s7, 20000

; li s2, 1
; li s3, 2
; li s4, 3
; li s5, 4
; li ra, 0xfefefe
; push ra, s7
; save s2, s5, s7
; restore s2, s5, s7
; pop ra, s7
; trace s2, s3, s4, s5
; trace ra, zero, zero, zero
; j [0]
pop ra, s7
push ra, s7    
save s2, s6, s7
call Main.main
pop ra, s7
trace s8, zero, zero, zero

end:
j end

; Interrupt Service Routines:
INITIALIZE_IVT:
	la a1, _IVT
	la a0, ISR_TTY_TRANSMIT
	swd a0, IVT_TTY_TRANSMIT(a1)
	la a0, ISR_RESET
    swd a0, IVT_RESET(a1)
	la a0, ISR_BUS_ERROR
    swd a0, IVT_BUS_ERROR(a1)
	la a0, ISR_ADDRESS_ERROR
    swd a0, IVT_ADDRESS_ERROR(a1)
	la a0, ISR_ILLEGAL_INSTRUCTION
    swd a0, IVT_ILLEGAL_INSTRUCTION(a1)
	la a0, ISR_DIVISION_ZERO
    swd a0, IVT_DIVISION_ZERO(a1)
	la a0, ISR_PRIVILEGE_VIOLATION
    swd a0, IVT_PRIVILEGE_VIOLATION(a1)
	la a0, ISR_PAGE_FAULT
    swd a0, IVT_PAGE_FAULT(a1)
	la a0, ISR_ACCESS_VIOLATION
    swd a0, IVT_ACCESS_VIOLATION(a1)
	la a0, ISR_KBD_CHARACTER
    swd a0, IVT_KBD_CHARACTER(a1)
	la a0, ISR_KBD_SCANCODE
    swd a0, IVT_KBD_SCANCODE(a1)
	la a0, ISR_TPS_LOAD_FINISHED
    swd a0, IVT_TPS_LOAD_FINISHED(a1)
	la a0, ISR_DISK_LOAD_FINISHED
    swd a0, IVT_DISK_LOAD_FINISHED(a1)
    la a0, ISR_TIMER_TIMEOUT
    swd a0, IVT_TIMER_TIMEOUT(a1)
    la a0, ISR_TIMER_INTERVAL
    swd a0, IVT_TIMER_INTERVAL(a1)
    la a0, ISR_VIDEO_REFRESH
    swd a0, IVT_VIDEO_REFRESH(a1)

    ret

ISR_TTY_TRANSMIT:               ; The tty interrupt will not be used, as the outer interpreter relies on polling
	sysret

ISR_RESET:                      ; The reset interrupt will jump to ABORT when fired
    li t0, 1
    trace t0, zero, zero, zero
    sysret

ISR_BUS_ERROR:
    li t0, 2
    trace t0, zero, zero, zero
    sysret

ISR_ADDRESS_ERROR:
    li t0, 3
    trace t0, zero, zero, zero
    sysret

ISR_ILLEGAL_INSTRUCTION:
    li t0, 4
    trace t0, zero, zero, zero
    sysret

ISR_DIVISION_ZERO:
    li t0, 5
    trace t0, zero, zero, zero
    sysret

ISR_PRIVILEGE_VIOLATION:
    li t0, 6
    trace t0, zero, zero, zero
    sysret

ISR_PAGE_FAULT:
    li t0, 7
    trace t0, zero, zero, zero
    sysret

ISR_ACCESS_VIOLATION:
    li t0, 8
    trace t0, zero, zero, zero
    sysret

ISR_KBD_CHARACTER:
    li t0, 9
    trace t0, zero, zero, zero
    sysret

ISR_KBD_SCANCODE:
    li t0, 10
    trace t0, zero, zero, zero
    sysret

ISR_TPS_LOAD_FINISHED:
    li t0, 11
    trace t0, zero, zero, zero
    sysret

ISR_DISK_LOAD_FINISHED:
    li t0, 12
    trace t0, zero, zero, zero
    sysret

ISR_TIMER_TIMEOUT:
    li t0, 13
    trace t0, zero, zero, zero
    sysret

ISR_TIMER_INTERVAL:
    li t0, 14
    trace t0, zero, zero, zero
    sysret

ISR_VIDEO_REFRESH:
    li t0, 15
    trace t0, zero, zero, zero
    sysret

#bankdef bss {
    #addr 30000
    #addr_end 40000
    #outp 8 * 30000
}
#include "Main.s"