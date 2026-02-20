#include <string.h>

char *strncat(char *s1, const char *s2, size_t n)
{
    size_t      len = strlen(s1);
    const char *sc;
    size_t      count;
    for (count = 0, sc = s2; count < n && *sc != '\0'; ++sc, ++count);
    memcpy(s1 + len, s2, count);
    s1[len + count] = '\0';
    return s1;
}
