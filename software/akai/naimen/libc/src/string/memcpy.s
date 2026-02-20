.globl memcpy
.text
.align 4
# void  *memcpy(void *s1, const void *s2, size_t n);
memcpy:
    mv x10, x12
    copy x13, x12, x14
    ret
