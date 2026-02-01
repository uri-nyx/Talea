# Minimal Firmware for the TALEA SYSTEM computer, v0.9

    .text
_start:

################################################################################
#                               SYSTEM INITIALIZATION:
#       1. Disable all interupts

    cli

#       2. Set stack pointer to the top of the FRIMWARE data section (0x10000)

    li x2, 0x10000

#       3. Initialize the IVT. All to NOT IMPLEMENTED except exception

    li      x11, 0x200
    la      x10, _start                     # RESET
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
    la      x10, _isr_not_implemented       # DEBUG STEP
    swd     x10, 36(x11)
    la      x10, _exception                 # OVERSPILL
    swd     x10, 40(x11)
    la      x10, _exception                 # UNDERSPILL
    swd     x10, 44(x11)

    la      x10, _isr_not_implemented       # SER RX
    swd     x10, 0x10*4(x11)
    la      x10, _isr_not_implemented       # KB CHARACTER
    swd     x10, 0x11*4(x11)
    la      x10, _isr_not_implemented       # KB SCANCODE
    swd     x10, 0x12*4(x11)
    la      x10, _isr_not_implemented       # TPS FINISH
    swd     x10, 0x13*4(x11)
    la      x10, _isr_not_implemented       # HCS FINISH
    swd     x10, 0x14*4(x11)
    la      x10, _isr_not_implemented       # TIMEOUT
    swd     x10, 0x15*4(x11)
    la      x10, _isr_not_implemented       # INTERVAL
    swd     x10, 0x16*4(x11)    
    la      x10, _isr_not_implemented       # VBLANK
    swd     x10, 0x17*4(x11)
    la      x10, _isr_not_implemented       # MOUSE PRESS
    swd     x10, 0x18*4(x11)
    la      x10, _isr_not_implemented       # TPS EJECTED
    swd     x10, 0x19*4(x11)
    la      x10, _isr_not_implemented       # TPS INSERTED
    swd     x10, 0x1A*4(x11)
    la      x10, _isr_not_implemented       # MUSIC NOTE END CH 0
    swd     x10, 0x1B*4(x11)

#       4. Transfer to C code. Interrupts are still disabled.

    .extern  firmware_start # _noreturn void firmware_start(void);
    
    call     firmware_start

#       5. Trap if somehow execution comes here

_halt:
    sti
    j       _halt
#
################################################################################
#
#                       INTERRUPT SERVICE ROUTINES
#
  
_isr_not_implemented:
    trace x0, x1, x2, x3
    j _halt
    sysret

_exception:
    trace x10, x9, x8, x7
    j _halt
    sysret
#                                               
################################################################################
    .globl _trace
_trace:
    trace   x12, x13, x14, x15
    ret
    .globl _sbd # extern void _sbd(u16 addr, u8 value)
_sbd:
    sbd     x13, 0(x12) #x12 addr, x13 u8 value
    ret

    .globl _shd # extern void _shd(u16 addr, u16 value);
_shd:
    shd     x13, 0(x12)
    ret

    .globl _swd # extern void _swd(u16 addr, u32 value);
_swd:
    swd     x13, 0(x12)
    ret

    .globl _lbud # extern u8 _lbud(u16 addr)
_lbud:
    lbud     x10, 0(x12) #x12 addr
    ret

    .globl _lhud # extern u16  _lhud(u16 addr);
_lhud:
    lhud    x10, 0(x12)
    ret

    .globl _lwd # extern u32 _lwd(u16 addr)
_lwd:
    lwd     x10, 0(x12) #x12 addr
    ret

    .globl _copydm # extern usize _copydm(u16 data_addr_src, void *buff_dest, usize sz);
_copydm:
    copydm x12, x13, x14
    mv x10, x14 # if interrupted, x14 will return actual bytes copied. //TODO: DOCUMENT THIS
    ret

    .globl _copymd # extern usize _copymd(void* buff_src, u16 data_addr_dest, usize sz);
_copymd:
    copymd x12, x13, x14
    mv x10, x14 # if interrupted, x14 will return actual bytes copied. //TODO: DOCUMENT THIS
    ret

    .globl memcpy # extern void* memcpy(void* dest, const void* src, size_t n);
memcpy:
    mv x10, x12
    copy x13, x12, x14
    ret