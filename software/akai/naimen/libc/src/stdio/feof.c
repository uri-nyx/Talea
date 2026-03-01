#include "xstdio.h"

int feof(FILE *stream)
{
    return (stream->_Mode & _MEOF);
}
