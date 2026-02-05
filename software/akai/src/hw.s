# Hardware wrappers for the Sirius ISA
    .align 4
    .text
    .globl _cli
_cli:
    cli
    ret

.globl _sti
_sti:
    sti
    ret

   .globl _trace
_trace:
    trace   x12, x13, x14, x15
    ret

    .globl _ssreg
_ssreg:
    ssreg x12
    ret

    .globl _gsreg
_gsreg:
    gsreg x10
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