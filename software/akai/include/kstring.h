#ifndef KSTRING_H
#define KSTRING_H

#include "akai_def.h"

extern void *memcpy(void *dest, const void *src, usize n);
extern void *memset(void *s, int c, usize n);
int          memcmp(const void *ptr1, const void *ptr2, usize count);
char        *strchr(const char *str, int character);
char        *strcpy(char *dst, const char *src);
usize        strlen(const char *str);
i32          strlen_max(const char *str, usize max);
char        *strrchr(const char *str, int character);

#endif /* KSTRING_H */
