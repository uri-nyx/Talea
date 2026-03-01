#include "xstdio.h"
#include "yfuns.h"
#include <stdlib.h>

int fclose(FILE *stream)
{
    int alfil = stream->_Mode & _MALFIL;
    int stat  = fflush(stream);

    if (stream->_Mode & _MALBUF) free(stream->_Buf);
    stream->_Buf = NULL;
    if (0 <= stream->_Handle && _Fclose(stream)) stat = EOF;
    if (stream->_Tmpnam) { /* remove temp file */
        if (remove(stream->_Tmpnam)) stat = EOF;
        free(stream->_Tmpnam), stream->_Tmpnam = NULL;
    }
    stream->_Mode  = 0;
    stream->_Next  = &stream->_Cbuf;
    stream->_Rend  = &stream->_Cbuf;
    stream->_Wend  = &stream->_Cbuf;
    stream->_Nback = 0;
    if (alfil) { /* find _Files[i] entry and free */
        size_t i;

        for (i = 0; i < FOPEN_MAX; ++i)
            if (_Files[i] == stream) { /* found entry */
                _Files[i] = NULL;
                break;
            }
        free(stream);
    }
    return (stat);
}
