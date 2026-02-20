#include <string.h>

char *strcat(char *s1, const char *s2)
{
    size_t len = strlen(s1);
    memcpy(s1 + len, s2, strlen(s2) + 1);
    return s1;
}
