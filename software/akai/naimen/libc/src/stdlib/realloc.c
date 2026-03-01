#include "xmem.h"
#include <string.h>

void *realloc(void *ptr, size_t size)
{
    _Cell *q;

    if (ptr == NULL) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (size < SIZE_CELL) {
        /* round up size */
        size = SIZE_CELL;
    } else if ((size = (size + _MEMBND) & ~_MEMBND) == 0 || size + CELL_OFF < size) {
        return NULL;
    }

    q = (_Cell *)((char *)ptr - CELL_OFF);

    /* erroneous call */
    if (q->_Size < SIZE_CELL || (q->_Size & _MEMBND) != 0) return NULL;

    if (size + CELL_OFF <= q->_Size - SIZE_CELL) {
        /* free excess space */
        _Cell *const new_q = (_Cell *)((char *)ptr + size);

        new_q->_Size = q->_Size - CELL_OFF - size;
        q->_Size     = size;
        free((char *)new_q + CELL_OFF);
        return ptr;
    } else if (size <= q->_Size) {
        /* leave cell alone */

        return ptr;
    } else {
        /* try to buy a larger cell */
        char *const new_p = (char *)malloc(size);

        if (new_p == NULL) return (NULL);
        memcpy(new_p, ptr, q->_Size);
        free(ptr);
        return new_p;
    }
}
