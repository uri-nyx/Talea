#ifndef _STDINT_H
#define _STDINT_H

typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed long  int32_t;

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;

typedef unsigned char  uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned long  uint_least32_t;

typedef unsigned long uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;

typedef signed long int_fast8_t;
typedef signed long int_fast16_t;
typedef signed long int_fast32_t;

typedef signed long   intmax_t;
typedef unsigned long uintmax_t;

typedef signed long   intptr_t;
typedef unsigned long uintptr_t;

#define INT8_MAX   127
#define INT8_MIN   (-128)
#define UINT8_MAX  255U
#define INT16_MAX  32767
#define INT16_MIN  (-32768)
#define UINT16_MAX 65535U
#define INT32_MAX  2147483647L
#define INT32_MIN  (-2147483647L - 1L)
#define UINT32_MAX 4294967295UL

#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST32_MAX INT32_MAX

#define INT_FAST8_MIN  INT32_MIN
#define INT_FAST8_MAX  INT32_MAX
#define INT_FAST16_MIN INT32_MIN
#define INT_FAST16_MAX INT32_MAX
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST32_MAX INT32_MAX

#define INTMAX_MIN  INT32_MIN
#define INTMAX_MAX  INT32_MAX
#define UINTMAX_MAX UINT32_MAX

#define INTPTR_MIN  (-2147483647L - 1L)
#define INTPTR_MAX  2147483647L
#define UINTPTR_MAX 4294967295UL

#define INT8_C(val)  val
#define INT16_C(val) val
#define INT32_C(val) val##L

#define UINT8_C(val)  val##U
#define UINT16_C(val) val##U
#define UINT32_C(val) val##UL

#endif /* _STDINT_H */
