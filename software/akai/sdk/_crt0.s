.globl main
.globl _exit
.globl _syscall
.globl _trace
.text
_start:
    call main
    mv x12, x10
    call _exit

_exit:
    mv x13, x12
    mv x12, x0 # SYSCALL_EXIT
    syscall x0, 0x40

_syscall:
    syscall x0, 0x40
    ret
    
_trace:
    trace x12, x13, x14, x15
    ret