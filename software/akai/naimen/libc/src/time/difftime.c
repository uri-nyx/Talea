#include <time.h>

double difftime(time_t t0, time_t t1)
{
    return (t0 <= t1) ? (double)(t1 - t0) : -(double)(t0 - t1);
}
