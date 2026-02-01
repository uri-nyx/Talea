# Akai, a minimal OS for the Taleä Computer System. Kernel entry point
# LOAD AT 0
    .text
_kstart:

################################################################################
#                               SYSTEM INITIALIZATION:
#       1. Disable all interupts

    cli

#       2. For The kernel is up to 128Kb (code+data+bss+stack), so we put stack 
#          at the 128Kb. The firmware should ensure we have at least that space
#          (and the user also should). (0x20000)

    li x2, 0x20000

#      3. Transfer to C code. The first argument is a pointer to the System Info
#         struct handed by the firmware. Its in x12, but since we are probably
#         overwriting its data section, lets use the copy in DATA memory. Pass 
#         the pointer to data memory in x12.

    .extern  kmain # _noreturn void kmain(void);

    li x12, 0x2000 # pointer to struct SystemInfo in data memory
    call     kmain

#       5. Trap if somehow execution comes here

_halt:
    sti
    j       _halt

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