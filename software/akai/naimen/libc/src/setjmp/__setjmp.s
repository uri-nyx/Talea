.align 4
.text
.globl __setjmp
__setjmp:
    sw x1,  0(x12)
    sw x2,  4(x12)
    sw x8,  8(x12)
    sw x9,  12(x12)
    sw x18, 16(x12)
    sw x19, 20(x12)
    sw x20, 24(x12)
    sw x21, 28(x12)
    sw x22, 32(x12)
    sw x23, 36(x12)
    sw x24, 40(x12)
    sw x25, 44(x12)
    sw x26, 48(x12)
    sw x27, 52(x12)

    mv x10, x0
    ret
