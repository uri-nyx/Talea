.text
    .globl _fillw # extern void *_fillw(void *s, int c, size_t n);
_fillw:
    # fill buff, n, fill
    mv   x10, x12
    fillw x12, x14, x13 
    ret

    .globl _copybck # extern void *_copybck(void *src, void *dest, usize sz);
_copybck:
    mv x10, x13
    copybck x12, x13, x14
    ret
