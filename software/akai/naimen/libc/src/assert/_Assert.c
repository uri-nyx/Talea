#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void _Assert(char *msg)
{
    fputs("Assertion failed: ", stderr);
    fputs(msg, stderr);
    fputc('\n', stderr);
    abort();
}
