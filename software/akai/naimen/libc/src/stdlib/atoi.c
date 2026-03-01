#include <stdlib.h>

int (atoi)(const char *nptr)
{
    return (int)_Stoul(nptr, NULL, 10);
}
