
#include "kstring.h"

int memcmp(const void *ptr1, const void *ptr2, usize count)
{
    u8   *p1 = (u8 *)ptr1;
    u8   *p2 = (u8 *)ptr2;
    int   res;
    usize i;

    for (i = 0; i < count; i++) {
        if ((res = (int)(p1[i] - p2[i]))) return res;
    }

    return 0;
}
 
char *strchr(const char *str, int character)
{
    char  c = character;
    char *s = (char *)str;

    while (*s != c) {
        if (*s == '\0') {
            return NULL;
        }
        s++;
    }

    return s;
}

char *strcpy(char *dst, const char *src)
{
    char *s = (char *)src;
    char *d = dst;

    while ((*d++ = *s++));

    return dst;
}

usize strlen(const char *str)
{
    char *s = (char *)str;
    while (*s++);
    return (usize)(s - str);
}

// Returns actual length (excluding null).
// Returns -1 if max is reached before a null terminator.
i32 strlen_max(const char *str, usize max)
{
    usize i;
    for (i = 0; i < max; i++) {
        if (str[i] == '\0') return (i32)i;
    }
    return -1;
}

char *strrchr(const char *str, int character)
{
    char *last = NULL;
    char  c    = (char)character;
    do {
        if (*str == c) last = (char *)str;
    } while (*str++);
    return last;
}