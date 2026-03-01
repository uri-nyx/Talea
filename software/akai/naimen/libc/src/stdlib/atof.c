#include <stdlib.h>

double(atof)(const char *nptr)
{
    return (_Stod(nptr, NULL));
}
