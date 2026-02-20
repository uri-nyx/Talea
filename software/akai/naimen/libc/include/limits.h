#ifndef _LIMITS_H
#define _LIMITS_H

#ifndef _YVALS
#include "yvals.h"
#endif

#define CHAR_BIT 8

/* char properties */
#if _CSIGN
#define CHAR_MAX 127
#define CHAR_MIN (-127 - _C2)
#else
#define CHAR_MAX 255
#define CHAR_MIN 0
#endif

/* int properties */
#if _ILONG
#define INT_MAX  2147483647
#define INT_MIN  (-2147483647 - _C2)
#define UINT_MAX 4294967295
#else
#error "no support for 16-bit int"
#endif

/* long properties */
#define LONG_MAX 2147483647L
#define LONG_MIN (-2147483647L - _C2)

/* multibyte properties */
#define MB_LEN_MAX _MBMAX

/* signed char properties */
#define SCHAR_MAX 127
#define SCHAR_MIN (-127 - _C2)

/* short properties */
#define SHRT_MAX 32767
#define SHRT_MIN (-32767 - _C2)

/* unsigned properties */
#define UCHAR_MAX 255
#define USHRT_MAX 65535
#define ULONG_MAX 4294967295UL

#endif /* _LIMITS_H */
