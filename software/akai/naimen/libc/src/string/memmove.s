
.globl memmove
.text
.align 4
# void  *memmove(void *s1, const void *s2, size_t n)
memmove:
    mv x10, x12
    ble  x12, x13, fwd
    copybck x13, x12, x14
    ret
fwd:    copy x13, x12, x14  
        ret