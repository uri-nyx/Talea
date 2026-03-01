#include <time.h>

extern long ak_time(void);

time_t     time(time_t *t) {
    time_t tim = ak_time();
    if (t) *t = tim;
    return tim;
}
