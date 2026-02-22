#ifndef _XMEM_H
#define _XMEM_H

#include <stddef.h>
#include <stdlib.h>

#ifndef _YVALS
#include <yvals.h>
#endif

#define CELL_OFF   (sizeof(size_t) + _MEMBND & ~_MEMBND)
#define SIZE_BLOCK 512
#define SIZE_CELL  ((sizeof(_Cell) + _MEMBND & ~_MEMBND) - CELL_OFF)

typedef struct _Cell {
    size_t        _Size;
    struct _Cell *_Next;
} _Cell;

typedef struct {
    _Cell **_Plast;
    _Cell  *_Head;
} _Altab;

extern _Altab _Aldata;

void *_Getmem(size_t bytes);

#endif /* _XMEM_H */
