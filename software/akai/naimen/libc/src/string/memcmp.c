
#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char  *p1 = (const unsigned char *)s1;
    const unsigned char  *p2 = (const unsigned char *)s2;
    int    res;
    size_t i;

    for (i = 0; i < n; i++) {
        if ((res = (int)p1[i] - (int)p2[i])) return res;
    }

    return 0;
}
