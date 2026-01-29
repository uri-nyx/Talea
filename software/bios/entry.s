    .text
_start:

################################################################################
#                               SYSTEM INITIALIZATION:
#       2. Disable all interupts

    cli

#       3. Set stack pointer to the top of the BIOS data section (0x10000)

    li x2, 0x10000

#       4. Initialize the IVT.

    li      x11, 0x200
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
    la      x10, _isr_not_implemented       # DEBUG STEP
    swd     x10, 36(x11)
    la      x10, _isr_not_implemented       # OVERSPILL
    swd     x10, 40(x11)
    la      x10, _isr_not_implemented       # UNDERSPILL
    swd     x10, 44(x11)

    la      x10, _isr_ser_rx                # SER RX
    swd     x10, 0x10*4(x11)
    la      x10, _isr_kbd_scancode          # KB CHARACTER
    swd     x10, 0x11*4(x11)
    la      x10, _isr_kbd_scancode          # KB SCANCODE
    swd     x10, 0x12*4(x11)
    la      x10, _isr_not_implemented       # TPS FINISH
    swd     x10, 0x13*4(x11)
    la      x10, _isr_not_implemented       # HCS FINISH
    swd     x10, 0x14*4(x11)
    la      x10, _isr_timer_timeout         # TIMEOUT
    swd     x10, 0x15*4(x11)
    la      x10, _isr_timer_interval        # INTERVAL
    swd     x10, 0x16*4(x11)    
    la      x10, _isr_video_refresh         # VBLANK
    swd     x10, 0x17*4(x11)
    la      x10, _isr_pointer_pressed       # MOUSE PRESS
    swd     x10, 0x18*4(x11)
    la      x10, _isr_tps_ejected           # TPS EJECTED
    swd     x10, 0x19*4(x11)
    la      x10, _isr_tps_inserted          # TPS INSERTED
    swd     x10, 0x1A*4(x11)
    la      x10, _isr_music_note_end        # MUSIC NOTE END CH 0
    swd     x10, 0x1B*4(x11)


    la      x10, _syscall_handler           # SYSCALL HANDLER AT VECTOR 0x20
    swd     x10, 0x20 * 4(x11)
    la      x10, _panic
    swd     x10, 0x21*4(x11)                # Paninc handler


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
    
    save x2, x0, x0 
    

    #call    bios_syscall_handler

   


    restore x11, x0, x0

    sti
    sysret

_isr_not_implemented:
    trace x0, x1, x2, x3
    j _halt
    sysret

_isr_ser_rx:
    trace x10, x8, x7, x6
    sysret

    .extern bios_reset
_isr_reset:
    #call    bios_reset
    li      x5, 0xff0000     # jump to the firmware
    jalr    x0, 0(x5)

    .extern bios_abort
_isr_address_error:
    li      x12, 0x3
    #call    bios_abort
    j _halt
    sysret

_isr_illegal_instruction:
    li      x12, 0x4
    #call    bios_abort
    j _halt
    sysret

_isr_division_zero:
    li      x12, 0x5
    #call    bios_abort
    j _halt
    sysret

_isr_privilege_violation:
    li      x12, 0x6
    #call    bios_abort
    sysret

    .extern bios_panic
_panic:
    
    save x2, x0, x0 
    

    #call    bios_panic

   

    restore x11, x0, x0

    sysret

    .extern bios_kbd_handler
_isr_kbd_scancode:
    
    save x2, x0, x0 
    

    call    bios_kbd_handler

   
    restore x0, x0, x0
    sysret

    .extern bios_timeout_handler
_isr_timer_timeout:
    
    save x2, x0, x0 
    

    call    bios_timeout_handler

   
    restore x0, x0, x0
    sysret

    .extern bios_interval_handler
_isr_timer_interval:
    
    save x2, x0, x0 
    

    call    bios_interval_handler

   

    restore x0, x0, x0
    sysret

    .extern bios_vblank_handler
_isr_video_refresh:
    
    save x2, x0, x0 
    

    call    bios_vblank_handler

   

    restore x0, x0, x0
    sysret

    .extern bios_pointer_pressed
_isr_pointer_pressed:
    
    save x2, x0, x0 
    

    #call    bios_pointer_pressed

   

    restore x0, x0, x0
    sysret

    .extern bios_tps_ejected_handler
_isr_tps_ejected:
    
    save x2, x0, x0 
    
    li      x12, 0x0

    #call    bios_tps_ejected_handler

   

    restore x0, x0, x0
    sysret

    .extern bios_tps_inserted_handler
_isr_tps_inserted:
    
    save x2, x0, x0 
    
    li      x12, 0x1

    #call    bios_tps_inserted_handler

   

    restore x0, x0, x0
    sysret

    .extern bios_music_note_end
_isr_music_note_end:
    
    save x2, x0, x0 
    

    #call    bios_music_note_end

   

    restore x0, x0, x0
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
    ret

    .globl _ssreg
_ssreg:
    ssreg x12
    ret

    .globl _cli
_cli:
    cli
    ret

    .globl _sti
_sti:
    sti
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

    .globl memset # extern void *memset(void *s, int c, size_t n)
memset:
    # fill buff, n, fill
    mv   x10, x12
    fill x12, x14, x13 
    ret

    .globl memseth # extern void *memseth(void *s, u16 c, size_t n)
memseth:
    # fillh buff, n, fill
    mv   x10, x12
    fillh x12, x14, x13 
    ret

    .globl memcpy # extern void* memcpy(void* dest, const void* src, size_t n);
memcpy:
    mv x10, x12
    copy x13, x12, x14
    ret