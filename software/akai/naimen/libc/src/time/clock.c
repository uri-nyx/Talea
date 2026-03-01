#include <time.h>

extern int ak_clock(unsigned int *, unsigned int *, unsigned int pid);

clock_t clock(void)
{
    unsigned int u, s;
    return (clock_t)ak_clock(&u, &s, 0);
}
