#include <stdlib.h>

long (atol)(const char *nptr)
{
    return (long)_Stoul(nptr, NULL, 10);
}
