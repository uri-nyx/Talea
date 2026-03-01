#include <time.h>

struct tm *(gmtime)(const time_t *tod)
{
    static struct tm tim;
    _Totm(tod, &tim);
    return &tim;
}
