#include <limits.h>
#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size)
{
    size_t n;
    char  *p;

    /* how do I check for overflow here? */
    if (nmemb == 0 || size == 0) return NULL;

    n = nmemb * size;

    if (n / size != nmemb) return NULL;

    p = malloc(n);

    if (p) memset(p, 0, n);
    return (void*)p;
}
