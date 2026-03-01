#include <stdlib.h>

long (labs)(long int j)
{
    return (j < 0) ? -j : j;
}
