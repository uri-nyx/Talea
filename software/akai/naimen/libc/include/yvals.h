#ifndef _YVALS
#define _YVALS

/* integers */
#define _C2    1 /* 1 = 2's complement */
#define _CSIGN 0 /* 0 = unsigned char */
#define _ILONG 1 /* 1 = 32-bit ints */

/* time */
#define _CPS 100 /* 10 ms ticks*/
#define _TBIAS 0

/* floating point */
#define _D0    1 /* big endian */
#define _DBIAS 0x7F
#define _DLONG 1 /* long double == double */
#define _DOFF  4

#define _FBIAS 0x7e
#define _FOFF  7
#define _FRND  1

#define _LBIAS 0x7F
#define _LOFF  15

/* errno */
#define _EDOM   33
#define _ERANGE 34
#define _EFPOS  35
#define _ERRMAX 36

/* stdio */
#define _FNAMAX 13
#define _FOPMAX 8
#define _TNAMAX 13

/* limits */
#define _MBMAX 1

/* alignement */
#define _ALIGNT \
    long // TODO: this will break because long long has alignement of 8, but the stack does not
#define _ALIGNB sizeof(_ALIGNT)
#define _MEMBND (_ALIGNB - 1)
#define _AUPBND (_ALIGNB - 1)
#define _ADNBND (_ALIGNB - 1)

/* setjmp */
#define _NSETJMP

/* NULL */
#define _NULL ((void *)0)

#define EOF -1

/* signal */
#define _SIGABRT
#define _SIGMAX

/* stdlib */
#define _EXFAIL 1

/* types */
typedef signed long    _Ptrdifft;
typedef unsigned long  _Sizet;
typedef unsigned short _Wchart;

#endif /* _YVALS */
