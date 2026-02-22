#include <stdlib.h>

div_t div(int numer, int denom)
{
    div_t val;

    val.quot = numer / denom;
    val.rem  = numer % denom;

#ifndef __SIRIUS__
    if (val.quot < 0 && 0 < val.rem) {
        val.quot += 1;
        val.rem -= denom;
    }
#endif

    return val;
}
