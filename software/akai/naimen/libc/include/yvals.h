#ifndef _YVALS
#define _YVALS

/* integers */
#define _C2    1 /* 1 = 2's complement */
#define _CSIGN 0 /* 0 = unsigned char */
#define _ILONG 1 /* 1 = 32-bit ints */

/* time */
#define _CPS
#define _TBIAS

/* floating point */
#define _D0
#define _DBIAS
#define _DLONG 1 /* long double == double */
#define _DOFF

#define _FBIAS
#define _FOFF
#define _FRND

#define _LBIAS
#define _LOFF

/* errno */
#define _EDOM   33
#define _ERANGE 34
#define _EFPOS3 35
#define _ERRMAX 36

/* stdio */
#define _FNAMAX
#define _FOPMAX
#define _TNAMAX

/* limits */
#define _MBMAX 1

/* alignement */
#define _ALIGNT long
#define _ALIGNB sizeof(_ALIGNT)
#define _MEMBND (_ALIGNB - 1)
#define _AUPBND (_ALIGNB - 1)
#define _ADNBND (_ALIGNB - 1)

/* setjmp */
#define _NSETJMP

/* NULL */
#define _NULL ((void *)0)

/* signal */
#define _SIGABRT
#define _SIGMAX

#ifndef EOF
#define EOF (-1)
#endif

/* stdlib */
#define _EXFAIL 1

/* types */
typedef signed long    _Ptrdifft;
typedef unsigned long  _Sizet;
typedef unsigned short _Wchart;

#endif /* _YVALS */
