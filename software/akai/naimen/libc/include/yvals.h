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
#define _EDOM
#define _EFPOS
#define _ERANGE
#define _ERRMAX

/* stdio */
#define _FNAMAX
#define _FOPMAX
#define _TNAMAX

/* limits */
#define _MBMAX 1

/* alignement */
#define _MEMBND
#define _AUPBND 3
#define _ADNBND 3

/* setjmp */
#define _NSETJMP

/* NULL */
#define _NULL ((void*)0)

/* signal */
#define _SIGABRT
#define _SIGMAX

/* types */
typedef signed long _Ptrdifft;
typedef unsigned long _Sizet;
typedef unsigned short _Wchart;


#endif /* _YVALS */
