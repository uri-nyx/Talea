#include <string.h>

size_t strxfrm(char *s1, const char *s2, size_t n)
{
    size_t len = strlen(s2);
    if (n > 0) strncpy(s1, s2, n);
    return len;
}
