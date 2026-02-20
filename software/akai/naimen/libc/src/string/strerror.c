#include <string.h>

char *strerror(int errnum)
{
    static char *s = "Unknown error (error.h not implemented)\n";
    return s;
}
