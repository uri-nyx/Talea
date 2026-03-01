#ifndef _STDARG_H
#define _STDARG_H

/* types */
typedef unsigned char *va_list;

#define _ALIGNT long // TODO: this will break eventually when we pass long long, because it has 8 byte alignement, BUT the stack has not
#define _ALIGNB sizeof(_ALIGNT)
#define _MEMBND (_ALIGNB - 1)
#define _AUPBND (_ALIGNB - 1)
#define _ADNBND (_ALIGNB - 1)

/* macros */

#define va_arg(ap, T) \
    (*(T*)(((ap) += _Bnd(T, _AUPBND)) - _Bnd(T, _ADNBND)))

#define va_end(ap) (void)0

#define va_start(ap, A) \
    (void)((ap) = (unsigned char*)&A + _Bnd(A, _AUPBND))

#define _Bnd(X, bnd) (sizeof(X) + (bnd) & ~(bnd))

#endif /* _STDARG_H */
