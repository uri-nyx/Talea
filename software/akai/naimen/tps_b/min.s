j _start
.byte #65
.byte #75
.byte #65
.byte #73
.byte #66
.byte #73
.byte #78
.byte #33
_start:
    mv %x12, %x0
    li %x13, #5
    syscall %x0, #0x40
_halt:
    j _halt
