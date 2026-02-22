#include "xmem.h"

void *ak_sbrk(unsigned int);

void *_Getmem(size_t bytes)
{
    char *p;

    if (bytes == 0 || bytes > (8 * 1024 * 1024)) return NULL;

    p = ak_sbrk(bytes + _MEMBND);
    if (p == (void *)-1) return NULL;

    if ((size_t)p & _MEMBND) {
        p += _ALIGNB - ((size_t)p & _MEMBND);
    }

    return (void *)p;
}
