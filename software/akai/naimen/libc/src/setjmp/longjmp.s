.align 4
.text
.globl longjmp
longjmp:
    # x12 = env, x13 = val

    # load saved registers
    lw x1,  0(x12)    # ra
    lw x2,  4(x12)    # sp
    lw x8,  8(x12)    # fp
    lw x9,  12(x12)
    lw x18, 16(x12)
    lw x19, 20(x12)
    lw x20, 24(x12)
    lw x21, 28(x12)
    lw x22, 32(x12)
    lw x23, 36(x12)
    lw x24, 40(x12)
    lw x25, 44(x12)
    lw x26, 48(x12)
    lw x27, 52(x12)

    # compute return value for setjmp
    mv x10, x13       # x10 = val
    beq x10, x0, __1  # if val == 0, force 1
    j __2
__1:
    li x10, 1
__2:
    ret 
