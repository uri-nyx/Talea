#include "xstdio.h"
#include "yfuns.h"

int fflush(FILE *stream)
{
    int            n;
    unsigned char *s;

    if (stream == NULL) { /* recurse on all streams */
        int nf, stat;

        for (stat = 0, nf = 0; nf < FOPEN_MAX; ++nf)
            if (_Files[nf] && fflush(_Files[nf]) < 0) stat = EOF;
        return (stat);
    }
    if (!(stream->_Mode & _MWRITE)) return (0);
    for (s = stream->_Buf; s < stream->_Next; s += n) { /* try to write buffer */
        n = _Fwrite(stream, s, stream->_Next - s);
        if (n <= 0) { /* report error and fail */
            stream->_Next = stream->_Buf;
            stream->_Wend = stream->_Buf;
            stream->_Mode |= _MERR;
            return (EOF);
        }
    }
    stream->_Next = stream->_Buf;
    stream->_Wend = stream->_Bend;
    return (0);
}
