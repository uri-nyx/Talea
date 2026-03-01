#include <time.h>

struct tm *(localtime)(const time_t *tod)
{
    return gmtime(tod);
}
