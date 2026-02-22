#include "xmem.h"

void free(void *ptr)
{
    _Cell *q;

    if (ptr == NULL) return;

    q = (_Cell *)((char *)ptr - CELL_OFF);

    if (q->_Size < SIZE_CELL || (q->_Size & _MEMBND) != 0) return;

    _Aldata._Plast = 0;

    if (_Aldata._Head == NULL || q < _Aldata._Head) {
        q->_Next      = _Aldata._Head;
        _Aldata._Head = q;
    } else {
        _Cell *qp;
        char  *qpp;

        for (qp = _Aldata._Head; qp->_Next && qp->_Next < q;) {
            qp = qp->_Next;
        }

        qpp = (char *)qp + CELL_OFF + qp->_Size;
        if ((char *)q < qpp) {
            return; /* erroneous call */
        } else if ((char *)q == qpp) {
            qp->_Size += CELL_OFF + q->_Size;
            q = qp;
        } else {
            q->_Next       = qp->_Next;
            qp->_Next      = q;
            _Aldata._Plast = &qp->_Next;
        }
    }

    if (q->_Next && (char *)q + CELL_OFF + q->_Size == (char *)q->_Next) {
        q->_Size += CELL_OFF + q->_Next->_Size;
        q->_Next = q->_Next->_Next;
    }
}
