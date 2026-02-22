#include "xmem.h"

_Altab _Aldata = { 0 };

static _Cell **findmem(size_t sz)
{
    _Cell *q, **qb;

    for (;;) {
        if ((qb = _Aldata._Plast) == NULL) {
            for (qb = &_Aldata._Head; *qb; qb = &(*qb)->_Next) {
                if (sz <= (*qb)->_Size) return qb;
            }
        } else {
            for (; *qb; qb = &(*qb)->_Next) {
                if (sz <= (*qb)->_Size) return qb;
            }

            q = *_Aldata._Plast;

            for (qb = &_Aldata._Head; *qb != q; qb = &(*qb)->_Next) {
                if (sz <= (*qb)->_Size) return qb;
            }
        }

        {
            size_t       bs;
            const size_t size = sz + CELL_OFF;

            for (bs = SIZE_BLOCK;; bs >>= 1) {
                if (bs < size) bs = size;
                if ((q = _Getmem(bs)) != NULL)
                    break;
                else if (bs == size)
                    return NULL;
            }

            q->_Size = bs - CELL_OFF;
            free((char *)q + CELL_OFF);
        }
    }
}

void *malloc(size_t size)
{
    _Cell *q, **qb;

    if (size < SIZE_CELL) {
        /* round up size */
        size = SIZE_CELL;
    } else if ((size = (size + _MEMBND) & ~_MEMBND) == 0 || size + CELL_OFF < size) {
        /* check for overflow */
        return NULL;
    }

    if ((qb = findmem(size)) == NULL) return NULL;

    q = *qb;
    if (q->_Size - SIZE_CELL < size + CELL_OFF) {
        *qb = q->_Next;
    } else {
        *qb          = (_Cell *)((char *)q + CELL_OFF + size);
        (*qb)->_Next = q->_Next;
        (*qb)->_Size = q->_Size - CELL_OFF - size;
        q->_Size     = size;
    }

    _Aldata._Plast = NULL;
    return ((char *)q + CELL_OFF);
}
