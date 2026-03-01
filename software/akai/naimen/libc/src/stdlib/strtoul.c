#include <stdlib.h>

unsigned long (strtoul)(const char *nptr, char **endptr, int base)
{
    return _Stoul(nptr, endptr, 10);
}
