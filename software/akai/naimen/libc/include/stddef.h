#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef _YVALS
#include "yvals.h"
#endif

/* macros */
#define NULL _NULL
#define offsetof(T, member) ((_Sizet)&((T*)0)->member)

/* types */
#ifndef _SIZET
#define _SIZET
typedef _Sizet size_t;
#endif

#ifndef _WCHART
#define _WCHART
typedef _Wchart wchar_t;
#endif

typedef _Ptrdifft ptrdiff_t;

#endif /* _STDDEF_H */
