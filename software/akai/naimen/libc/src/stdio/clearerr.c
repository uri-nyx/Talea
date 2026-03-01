#include "xstdio.h"

void clearerr(FILE *stream)
{
    if (stream->_Mode & (_MOPENR | _MOPENW)) stream->_Mode &= ~(_MEOF | _MERR);
}
