# Akai, a minimal OS for the Taleä Computer System. Kernel entry point
# LOAD AT 0
    .align 4
    .text
    .globl _kstart
_kstart:

################################################################################
#                               SYSTEM INITIALIZATION:
#       1. Disable all interupts and clear bss until 0x20000.

    cli

    .align 4
    .bss
    .globl _bss_start
    _bss_start:
    .data
    .globl _data_start
    _data_start:
    .text
    .align 4

#       2. For The kernel is up to 512K (code+data+bss+stack+tables). The firmware 
#          should ensure we have at least that space (and the user also should).

    li x2, 0X07A000

#       3. Set the exceptions to the exception handler and the interrupts to 
#          the interrupt handler. Set the syscalls to vector 0x40.

    li      x11, 0x200
    la      x10, _exception                 # RESET
    swd     x10, 0(x11)
    la      x10, _exception                 # BUS ERROR
    swd     x10, 8(x11)
    la      x10, _exception                 # ADDRESS ERROR
    swd     x10, 12(x11)
    la      x10, _exception                 # ILLEGAL INSTRUCTION
    swd     x10, 16(x11)
    la      x10, _exception                 # DIVISION BY ZERO
    swd     x10, 20(x11)
    la      x10, _exception                 # PRIVILEGE VIOLATION
    swd     x10, 24(x11)
    la      x10, _exception                 # PAGE FAULT
    swd     x10, 28(x11)
    la      x10, _exception                 # ACCESS VIOLATION
    swd     x10, 32(x11)
    la      x10, _exception                 # DEBUG STEP
    swd     x10, 36(x11)
    la      x10, _exception                 # OVERSPILL
    swd     x10, 40(x11)
    la      x10, _exception                 # UNDERSPILL
    swd     x10, 44(x11)

    la      x10, _interrupt                 # SER RX
    swd     x10, 0x10*4(x11)
    la      x10, _interrupt                 # KB CHARACTER
    swd     x10, 0x11*4(x11)
    la      x10, _interrupt                 # KB SCANCODE
    swd     x10, 0x12*4(x11)
    la      x10, _interrupt                 # TPS FINISH
    swd     x10, 0x13*4(x11)
    la      x10, _interrupt                 # HCS FINISH
    swd     x10, 0x14*4(x11)
    la      x10, _interrupt                 # TIMEOUT
    swd     x10, 0x15*4(x11)
    la      x10, _interrupt                 # INTERVAL
    swd     x10, 0x16*4(x11)    
    la      x10, _interrupt                 # VBLANK
    swd     x10, 0x17*4(x11)
    la      x10, _interrupt                 # MOUSE PRESS
    swd     x10, 0x18*4(x11)
    la      x10, _interrupt                 # TPS EJECTED
    swd     x10, 0x19*4(x11)
    la      x10, _interrupt                 # TPS INSERTED
    swd     x10, 0x1A*4(x11)
    la      x10, _interrupt                 # MUSIC NOTE END CH 0
    swd     x10, 0x1B*4(x11)

    la      x10, _syscall
    swd     x10, 0x40*4(x11)    

#      4. Transfer to C code. The first argument is a pointer to the System Info
#         struct handed by the firmware. Its in x12, but since we are probably
#         overwriting its data section, lets use the copy in DATA memory. Pass 
#         the pointer to data memory in x12.

    .extern  kmain # _noreturn void kmain(void);

    li x12, 0x2000 # pointer to struct SystemInfo in data memory
    call     kmain

#       5. Trap if somehow execution comes here
.globl _halt
_halt:
    sti
    j       _halt



################################################################################

    .globl akai_exception
_exception:
    save x2, x0, x0

    # save user pc and status
    lw x5, 4(x2) # user pc
    swd x5, 0x1000(x0) # stora at DATA[0x1000]
    lw x5, 0(x2) # user status
    swd x5, 0x1004(x0) # store at DATA[0x1004]

    lbud x12, 288(x0)   # REG_SYSTEM_EXCEPTION
    lwd  x13, 289(x0)   # REG_SYSTEM_FAULT_ADDR 
    call    akai_exception

    restore x0, x0, x0
    swd x5, 0x1010(x0)
    lwd x5, 0x1008(x0)
    trace x5, x0, x0, x0
    sw x5, 4(x2)
    lwd x5, 0x100C(x0)
    trace x5, x0, x0, x0
    sw x5, 0(x2)
    lwd x5, 0x1010(x0)
    sysret

 .globl akai_interrupt
_interrupt:
    cli
    save x2, x0, x0 

    # save user pc and status
    lw x5, 4(x2) # user pc
    swd x5, 0x1000(x0) # stora at DATA[0x1000]
    trace x0, x0, x0, x5
    lw x5, 0(x2) # user status
    trace x0, x0, x5, x2
    swd x5, 0x1004(x0) # store at DATA[0x1004]

    lbud x12, 441(x0)   # REG_SYSTEM_INTERRUPT (last interrupt ack)

    call    akai_interrupt

    restore x0, x0, x0

    swd x5, 0x1010(x0)
    lwd x5, 0x1008(x0)
    trace x5, x0, x0, x0
    sw x5, 4(x2)
    lwd x5, 0x100C(x0)
    trace x5, x0, x0, x0
    sw x5, 0(x2)
    lwd x5, 0x1010(x0)

    sti
    sysret

 .globl akai_syscall
_syscall:
    save x2, x12, x0     # TODO: define syscall abi

    # save user pc and status
    lw x5, 4(x2) # user pc
    trace x2, x0, x0, x5
    swd x5, 0x1000(x0) # stora at DATA[0x1000]
    lw x5, 0(x2) # user status
    trace x2, x0, x5, x0
    swd x5, 0x1004(x0) # store at DATA[0x1004]

    call    akai_syscall
    
    restore x10, x0, x0
    swd x5, 0x1010(x0)
    lwd x5, 0x1008(x0)
    trace x5, x0, x0, x0
    sw x5, 4(x2)
    lwd x5, 0x100C(x0)
    trace x5, x0, x0, x0
    sw x5, 0(x2)
    lwd x5, 0x1010(x0)
    sysret

################################################################################

    .globl _load_init
_load_init:
    swd x14, 0x101C(x0)

    save x2, x12, x13

    lwd x14, 0x101C(x0)
    push x12, x2 # pushes pc
    push x13, x2 # pushes status

    lw x1, 1*4(x14)
    lw x3, 3*4(x14)
    lw x4, 4*4(x14)
    lw x5, 5*4(x14)
    lw x6, 6*4(x14)
    lw x7, 7*4(x14)
    lw x8, 8*4(x14)
    lw x9, 9*4(x14)
    lw x10, 10*4(x14)
    lw x11, 11*4(x14)
    lw x12, 12*4(x14)
    lw x13, 13*4(x14)
    lw x15, 15*4(x14)
    lw x16, 16*4(x14)
    lw x17, 17*4(x14)
    lw x18, 18*4(x14)
    lw x19, 19*4(x14)
    lw x20, 20*4(x14)
    lw x21, 21*4(x14)
    lw x22, 22*4(x14)
    lw x23, 23*4(x14)
    lw x24, 24*4(x14)
    lw x25, 25*4(x14)
    lw x26, 26*4(x14)
    lw x27, 27*4(x14)
    lw x28, 28*4(x14)
    lw x29, 29*4(x14)
    lw x30, 30*4(x14)
    lw x31, 31*4(x14)
    lw x14, 14*4(x14)

    sysret  # sp becomes usp here

    .globl _load_and_switch
_load_and_switch:
    swd x14, 0x101C(x0)

    restore x2, x12, x13

    lwd x14, 0x101C(x0)
    push x12, x2 # pushes pc
    push x13, x2 # pushes status

    lw x1, 1*4(x14)
    lw x3, 3*4(x14)
    lw x4, 4*4(x14)
    lw x5, 5*4(x14)
    lw x6, 6*4(x14)
    lw x7, 7*4(x14)
    lw x8, 8*4(x14)
    lw x9, 9*4(x14)
    lw x10, 10*4(x14)
    lw x11, 11*4(x14)
    lw x12, 12*4(x14)
    lw x13, 13*4(x14)
    lw x15, 15*4(x14)
    lw x16, 16*4(x14)
    lw x17, 17*4(x14)
    lw x18, 18*4(x14)
    lw x19, 19*4(x14)
    lw x20, 20*4(x14)
    lw x21, 21*4(x14)
    lw x22, 22*4(x14)
    lw x23, 23*4(x14)
    lw x24, 24*4(x14)
    lw x25, 25*4(x14)
    lw x26, 26*4(x14)
    lw x27, 27*4(x14)
    lw x28, 28*4(x14)
    lw x29, 29*4(x14)
    lw x30, 30*4(x14)
    lw x31, 31*4(x14)
    lw x14, 14*4(x14)

    sysret  # sp becomes usp here

    .globl _switch
_switch:
    swd x12, 0x1008(x0) # store pc at DATA[0x1008]
    swd x13, 0x100C(x0) # store status at DATA[0x100C]

    trace x1, x30, x12, x13

    restore x0, x0, x0

    trace x1, x30, x12, x13


    swd x5, 0x1010(x0)
    lwd x5, 0x1008(x0)
    sw x5, 4(x2)
    lwd x5, 0x100C(x0)
    sw x5, 0(x2)
    lwd x5, 0x1010(x0)
    sysret

    .globl _load_window # extern void _load_window(u8 wp, void *buf);
_load_window:
    sbd x12, 303(x0) # _sbd(REG_SYSTEM_WIN_SEL, wp);
    li x12, 1
    sbd x12, 304(x0) # _sbd(REG_SYSTEM_WIN_OP, TALEA_SYSTEM_WIN_OP_STORE);

    lwd x12, 1*4+305(x0)
    sw x12, 1*4(x13)
    lwd x12, 2*4+305(x0)
    sw x12, 2*4(x13)
    lwd x12, 3*4+305(x0)
    sw x12, 3*4(x13)
    lwd x12, 4*4+305(x0)
    sw x12, 4*4(x13)
    lwd x12, 5*4+305(x0)
    sw x12, 5*4(x13)
    lwd x12, 6*4+305(x0)
    sw x12, 6*4(x13)
    lwd x12, 7*4+305(x0)
    sw x12, 7*4(x13)
    lwd x12, 8*4+305(x0)
    sw x12, 8*4(x13)
    lwd x12, 9*4+305(x0)
    sw x12, 9*4(x13)
    lwd x12, 10*4+305(x0)
    sw x12, 10*4(x13)
    lwd x12, 12*4+305(x0)
    sw x12, 12*4(x13)    
    lwd x12, 11*4+305(x0)
    sw x12, 11*4(x13)
    lwd x12, 13*4+305(x0)
    sw x12, 13*4(x13)
    lwd x12, 14*4+305(x0)
    sw x12, 14*4(x13)
    lwd x12, 15*4+305(x0)
    sw x12, 15*4(x13)
    lwd x12, 16*4+305(x0)
    sw x12, 16*4(x13)
    lwd x12, 17*4+305(x0)
    sw x12, 17*4(x13)
    lwd x12, 18*4+305(x0)
    sw x12, 18*4(x13)
    lwd x12, 19*4+305(x0)
    sw x12, 19*4(x13)
    lwd x12, 20*4+305(x0)
    sw x12, 20*4(x13)
    lwd x12, 21*4+305(x0)
    sw x12, 21*4(x13)
    lwd x12, 22*4+305(x0)
    sw x12, 22*4(x13)
    lwd x12, 23*4+305(x0)
    sw x12, 23*4(x13)
    lwd x12, 24*4+305(x0)
    sw x12, 24*4(x13)
    lwd x12, 25*4+305(x0)
    sw x12, 25*4(x13)
    lwd x12, 26*4+305(x0)
    sw x12, 26*4(x13)
    lwd x12, 27*4+305(x0)
    sw x12, 27*4(x13)
    lwd x12, 28*4+305(x0)
    sw x12, 28*4(x13)
    lwd x12, 29*4+305(x0)
    sw x12, 29*4(x13)
    lwd x12, 30*4+305(x0)
    sw x12, 30*4(x13)
    lwd x12, 31*4+305(x0)
    sw x12, 31*4(x13)

    ret

    .globl memcpy # extern void* memcpy(void* dest, const void* src, size_t n);
memcpy:
    mv x10, x12
    copy x13, x12, x14
    ret

    .globl memset # extern void *memset(void *s, int c, size_t n);
memset:
    # fill buff, n, fill
    mv   x10, x12
    fill x12, x14, x13 
    ret
