#ifndef _CTYPE_H
#define _CTYPE_H

/* character classes */
#define _XA 0x200 /* extra alpha */
#define _XS 0x100 /* extra space */
#define _BB 0x080 /* BEL, BS ... */
#define _CN 0x040 /* \r, \f, \t, \n, \v */
#define _DI 0x020 /* '0' - '9'   */
#define _LO 0x010 /* 'a' - 'z'   */
#define _PU 0x008 /* punctuation */
#define _SP 0x004 /* sapces      */
#define _UP 0x002 /* 'A' - 'Z'   */
#define _XD 0x001 /* hex digits  */

/* declarations */

int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);
int tolower(int);
int toupper(int);

extern const signed short *_Ctype, *_Tolower, *_Toupper;

/* macro overrides */
#define isalnum(c)  (_Ctype[(int)(c)] & (_DI | _UP | _LO | _XA))
#define isalpha(c)  (_Ctype[(int)(c)] & (_UP | _LO | _XA))
#define iscntrl(c)  (_Ctype[(int)(c)] & (_BB | _CN))
#define isdigit(c)  (_Ctype[(int)(c)] & _DI)
#define isgraph(c)  (_Ctype[(int)(c)] & (_DI | _LO | _PU | _UP | _XA))
#define islower(c)  (_Ctype[(int)(c)] & _LO)
#define isprint(c)  (_Ctype[(int)(c)] & ((_DI | _LO | _PU | _SP | _UP | _XA)))
#define ispunct(c)  (_Ctype[(int)(c)] & _PU)
#define isspace(c)  (_Ctype[(int)(c)] & (_CN | _SP | _XS))
#define isupper(c)  (_Ctype[(int)(c)] & _UP)
#define isxdigit(c) (_Ctype[(int)(c)] & _XD)

#define tolower(c) _Tolower[(int)(c)]
#define toupper(c) _Toupper[(int)(c)]

#endif /* _CTYPE_H */
