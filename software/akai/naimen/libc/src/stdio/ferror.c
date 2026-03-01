#include "xstdio.h"

int ferror(FILE *stream)
{
    return (stream->_Mode & _MERR);
}
