#include <stdio.h>
#include <stdlib.h>

extern void ak_exit(int);
void        exit(int status)
{
    size_t i;
    for (i = 0; i < FOPEN_MAX; ++i)
        if (_Files[i]) fclose(_Files[i]);

    ak_exit(status);
}
