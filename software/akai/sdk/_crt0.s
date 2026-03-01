.globl _program
.globl _exit
.globl _syscall
.globl main
.globl _trace
.text
_start:
    li x5, 0x11223344
    li x6, 0x55667788
    trace x5, x6, x2, x0
    call _program
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

__float64_add:
__float64_sub:
__float64_mul:
__float64_div:
__float64_neg:
__float64_eq:
__float64_ne:
__float64_lt:
__float64_le:
__float64_gt:
__float64_ge:
__int32_to_float64:
__float64_to_int32:
__int64_to_float64:
__float64_to_int64:
__float64_to_float:
__float_to_float64:
__add64:
__sub64:
__and64:
__or64:
__xor64:
__not64:
__neg64:
    mv x10, x0
    mv x11, x0
    ret
