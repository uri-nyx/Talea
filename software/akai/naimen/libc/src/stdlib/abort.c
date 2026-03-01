#include <stdlib.h>

extern void ak_abort(void);

void abort(void)
{
    ak_abort();
}
