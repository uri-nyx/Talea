
#include <string.h>

char *strcpy(char *s1, const char *s2)
{
    return (char *)memcpy(s1, s2, strlen(s2) + 1);
}
