#include <string.h>

char *strrchr(const char *s, int c)
{
    const char  cc = c;
    const char *sc;

    for (sc = NULL;; ++s) {
        if (*s == cc) sc = s;
        if (*s == '\0') return (char *)sc;
    }
}
