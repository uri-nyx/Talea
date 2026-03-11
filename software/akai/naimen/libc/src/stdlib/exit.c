#include <stdio.h>
#include <stdlib.h>

/* macros */
#define NATS 32

/* static data */
void (*_Atfuns[NATS])(void) = { 0 };
size_t _Atcount             = { NATS };

extern void ak_exit(int);

void exit(int status)
{
    size_t i;

    while (_Atcount < NATS) (*_Atfuns[_Atcount++])();

    for (i = 0; i < FOPEN_MAX; ++i)
        if (_Files[i]) fclose(_Files[i]);

    ak_exit(status);
}
