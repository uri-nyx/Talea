#include <akai.h>
#include <stdio.h>

#include "../stdio/xstdio.h"

static int isatty(unsigned int proxy)
{
    int res = ak_dev_ctl(proxy, PX_GET_DEV, NULL, 0);
    if (res < A_OK) return 0;

    return (res == (PDEV_TYPE_HW | DEV_TEXTBUFFER)) || (res == (PDEV_TYPE_HW | DEV_SERIAL));
}

void _Initio(void)
{
    static unsigned char stdout_buf[128];       /* stdin: mark as open for reading */
    stdin->_Mode |= _MOPENR;                    /* stdout: mark as open for writing */
    stdout->_Mode |= _MOPENW | _MWRITE;         /* stderr: always unbuffered */
    stderr->_Mode |= _MOPENW | _MNBF | _MWRITE; /* stderr buffer setup */
    stderr->_Buf  = &stderr->_Cbuf;
    stderr->_Next = &stderr->_Cbuf;
    stderr->_Rend = &stderr->_Cbuf;
    stderr->_Wend = &stderr->_Cbuf;
    stderr->_Bend = &stderr->_Cbuf + 1; 

    /* detect terminal */
    if (isatty(PDEV_STDOUT)) stdout->_Mode |= _MLBF;
    
    /* unbuffered */
    if (stdout->_Mode & _MNBF) {
        stdout->_Buf  = &stdout->_Cbuf;
        stdout->_Next = &stdout->_Cbuf;
        stdout->_Rend = &stdout->_Cbuf;
        stdout->_Wend = &stdout->_Cbuf;
        stdout->_Bend = &stdout->_Cbuf + 1;
        return;
    } 
    
    /* line-buffered */
    if (stdout->_Mode & _MLBF) {
        stdout->_Buf  = stdout_buf;
        stdout->_Next = stdout_buf;
        stdout->_Rend = stdout_buf;
        stdout->_Wend = stdout_buf + sizeof(stdout_buf);
        stdout->_Bend = stdout_buf + sizeof(stdout_buf);
        return;
    }

    /* fully buffered */
    stdout->_Buf  = stdout_buf;
    stdout->_Next = stdout_buf;
    stdout->_Rend = stdout_buf;
    stdout->_Wend = stdout_buf + sizeof(stdout_buf);
    stdout->_Bend = stdout_buf + sizeof(stdout_buf);
}
