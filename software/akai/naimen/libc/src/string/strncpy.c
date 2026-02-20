#include <string.h>

char *strncpy(char *s1, const char *s2, size_t n)
{
    const char *sc;
    size_t      count;
    for (count = 0, sc = s2; count < n && *sc != '\0'; ++sc, ++count);

    memcpy(s1, s2, count);
    if (count < n) {
        memset(s1 + count, '\0', n - count);
    }

    return s1;
}
