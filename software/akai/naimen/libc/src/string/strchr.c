#include <string.h>

char *strchr(const char *s, int c)
{
    char cc = c;

    for (; *s != cc; ++s) {
        if (*s == '\0') {
            return NULL;
        }
    }

    return (char *)s;
}
