#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

long strtol(const char *nptr, char **endptr, int base)
{
    const char   *sc;
    char         *se, sign;
    unsigned long x;

    if (endptr == NULL) endptr = &se;

    for (sc = nptr; isspace(*sc); ++sc);

    if (*sc == '-' || *sc == '+') {
        sign = *sc++;
    } else {
        sign = '+';
    }

    x = _Stoul(sc, endptr, base);

    if (sc == *endptr) *endptr = (char *)nptr;
    if (nptr == *endptr && x != 0 || sign == '+' && LONG_MAX < x ||
        sign == '-' && -(unsigned long)LONG_MIN < x) {
        errno = ERANGE;
        return sign == '-' ? LONG_MIN : LONG_MAX;
    } else {
        return (long)(sign == '-' ? -x : x);
    }
}
