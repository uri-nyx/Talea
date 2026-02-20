
.globl memset
.text
.align 4
# void  *memset(void *s, int c, size_t n)
memset:
    # fill buff, n, fill
    mv   x10, x12
    fill x12, x14, x13 
    ret
