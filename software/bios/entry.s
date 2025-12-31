    .text
_start:

################################################################################
#                               SYSTEM INITIALIZATION:
#       1. Set status register as: supervisor, intterupt enabled, mmu disabled
#          priority 2, ivt at 0xf800

    li      x10, 3421499392
    ssreg   x10

#       2. Disable all interupts

    cli

#       3. Set stack pointer to the top of memory - 8 K. We query the SYSTEM 
#          DEVICE at port MEMSIZE/FLASH. Reading from it gives amount of memory
#          in MB ( 1024*1024 bytes).

    lbu     x2, 0xF0(x0)
    muli    x2, x2, 1024
    muli    x2, x2, 1024
    addi    x2, x2, -32768

#       4. Initialize the IVT.

    li      x11, 0xf800
    la      x10, _isr_reset                 # RESET
    swd     x10, 0(x11)
    la      x10, _isr_not_implemented       # BUS ERROR
    swd     x10, 8(x11)
    la      x10, _isr_address_error         # ADDRESS ERROR
    swd     x10, 12(x11)
    la      x10, _isr_illegal_instruction   # ILLEGAL INSTRUCTION
    swd     x10, 16(x11)
    la      x10, _isr_division_zero         # DIVISION BY ZERO
    swd     x10, 20(x11)
    la      x10, _isr_privilege_violation   # PRIVILEGE VIOLATION
    swd     x10, 24(x11)
    la      x10, _isr_not_implemented       # PAGE FAULT
    swd     x10, 28(x11)
    la      x10, _isr_not_implemented       # ACCESS VIOLATION
    swd     x10, 32(x11)
    la      x10, _isr_tty_transmit          # TTY TRANSMIT #TODO: REAME THIS ISR
    swd     x10, 0xa*4(x11)
    la      x10, _isr_kbd_scancode          # KB CHARACTER
    swd     x10, 44(x11)
    la      x10, _isr_kbd_scancode          # KB SCANCODE
    swd     x10, 48(x11)
    la      x10, _isr_not_implemented       # TPS FINISH
    swd     x10, 52(x11)
    la      x10, _isr_not_implemented       # HCS FINISH
    swd     x10, 56(x11)
    la      x10, _isr_timer_timeout         # TIMEOUT
    swd     x10, 60(x11)
    la      x10, _isr_timer_interval        # INTERVAL
    swd     x10, 64(x11)    
    la      x10, _isr_video_refresh         # VBLANK
    swd     x10, 68(x11)
    la      x10, _isr_pointer_pressed       # MOUSE PRESS
    swd     x10, 72(x11)
    la      x10, _isr_tps_ejected           # TPS EJECTED
    swd     x10, 76(x11)
    la      x10, _isr_tps_inserted          # TPS INSERTED
    swd     x10, 80(x11)
    la      x10, _isr_music_note_end        # MUSIC NOTE END CH 0
    swd     x10, 84(x11)
    la      x10, _isr_music_note_end        # MUSIC NOTE END CH 1
    swd     x10, 88(x11)
    la      x10, _isr_music_note_end        # MUSIC NOTE END CH 2
    swd     x10, 92(x11)
    la      x10, _isr_music_note_end        # MUSIC NOTE END CH 3
    swd     x10, 96(x11)
    la      x10, _panic
    swd     x10, 100(x11)                   # Paninc handler


    la      x10, _syscall_handler           # SYSCALL HANDLER AT VECTOR 0x20
    swd     x10, 0x20 * 4(x11)


#       5. Transfer to C code. Interrupts are still disabled.

    .extern  bios_start # _noreturn void bios_start(void);
    sti
    call     bios_start

#       6. Trap if somehow execution comes here

_halt:
    sti
    j       _halt
#
################################################################################
#
#                       INTERRUPT SERVICE ROUTINES
#
    .extern bios_syscall_handler
_syscall_handler:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_syscall_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48

    swd     x11, 0x110(x0)


    mv      x31, x2
    restore x1, x30, x31

    lwd     x11, 0x110(x0)

    sti
    sysret

_isr_not_implemented:
    trace x0, x1, x2, x3
    sysret

_isr_tty_transmit:
    trace x10, x8, x7, x6
    sysret

    .extern bios_reset
_isr_reset:
    #call    bios_reset
    li      x5, 0xffe000     # jump to the firmware
    jalr    x0, 0(x5)

    .extern bios_abort
_isr_address_error:
    li      x12, 0x3
    #call    bios_abort
    sysret

_isr_illegal_instruction:
    li      x12, 0x4
    #call    bios_abort
    sysret

_isr_division_zero:
    li      x12, 0x5
    #call    bios_abort

_isr_privilege_violation:
    li      x12, 0x6
    #call    bios_abort

    .extern bios_panic
_panic:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_panic

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48

    swd     x11, 0x110(x0)


    mv      x31, x2
    restore x1, x30, x31

    lwd     x11, 0x110(x0)

    sti
    sysret

    .extern bios_kbd_handler
_isr_kbd_scancode:
    #TODO: Rework save and restore so it leaves the stack in this exact state
    # with only saveall or something
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    call    bios_kbd_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48

    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_timeout_handler
_isr_timer_timeout:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_timeout_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48

    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_interval_handler
_isr_timer_interval:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_interval_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_vblank_handler
_isr_video_refresh:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_vblank_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_pointer_pressed
_isr_pointer_pressed:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_pointer_pressed

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_tps_ejected_handler
_isr_tps_ejected:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)
    li      x12, 0x0

    #call    bios_tps_ejected_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_tps_inserted_handler
_isr_tps_inserted:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)
    li      x12, 0x1

    #call    bios_tps_inserted_handler

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret

    .extern bios_music_note_end
_isr_music_note_end:
    cli
    mv      x31, x2
    save    x1, x30, x31
    mv      x2, x31

    addi    x2,x2,-48
    sw      x8,44(x2)
    addi    x8,x2,32
    sw      x1,24(x2)

    #call    bios_music_note_end

    lw      x1,24(x2)
    lw      x8,44(x2)
    addi    x2,x2,48


    mv      x31, x2
    restore x1, x30, x31
    sti
    sysret
#
#
################################################################################
#           MISCELLANEOUS FUNCTIONS (to be called from C)
#
    .globl _trace
_trace:
    trace   x12, x13, x14, x15
    ret

    .globl _trace_sreg
_trace_sreg:
    gsreg   x5
    trace   x5, x0,x0,x0
    ret

    .globl _sbd # extern void _sbd(u16 addr, u8 value)
_sbd:
    sbd     x13, 0(x12) #x12 addr, x13 u8 value
    ret

    .globl _shd # extern void _shd(u16 addr, u16 value);
_shd:
    shd     x13, 0(x12)
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

    .globl memset # extern void *memset(void *s, int c, size_t n)
memset:
    # fill buff, n, fill
    mv   x10, x12
    fill x12, x14, x13 
    ret
    .globl memcpy # extern void* memcpy(void* dest, const void* src, size_t n);
memcpy:
    mv x10, x12
    copy x13, x12, x14