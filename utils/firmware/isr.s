#bankdef text {
    #addr 0
    #addr_end 10000
    #outp 8 * 0
}
#bankdef bss {
    #addr 30000
    #addr_end 40000
    #outp 8 * 30000
}
#bankdef data {
    #addr 40000
    #addr_end 50000
    #outp 8 * 40000
}
#bank text

CB = 0xed_17_f0 ; 15538160
FB = 0xed_3f_f0 ; 15548400

li  a0, 0b1_1_0_010_111110_11111111_000000000000
ssreg a0                ; supervisor, intterupt enabled, mmu disabled
                        ; priority 2, ivt at 0xf800, pdt at 0xff00
jal ra, INITIALIZE_IVT


;26 Initialize video
li t0, 0x2 ; set mode
li t1, 0x5 ; combined text + graphics mode
V_COMMAND = 0x10 + 0x0
V_DATAH = 0x10 + 0x1
sbd t1, V_DATAH(zero)
sbd t0, V_COMMAND(zero)

li t0, CB
li t1, 32
li t2, 2400
fill t0, t2, t1

li s2, 10000
li s8, 20000
save s2, s7, s8
subi s3, s2, 4
call Test.main
pop ra, s8
save s2, s7, s8
call Test.main
pop ra, s8
trace s9, zero, zero, zero

end:
j end

bios:
    .trace:
        trace s8, zero, zero, zero
        pop s8, s2
        ret


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


#include "jack.s"