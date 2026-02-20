#include <string.h>

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char  u = c;
    const unsigned char *su;

    for (su = s; 0 < n; ++su, --n) {
        if (*su == u) return (void *)su;
    }

    return NULL;
}
