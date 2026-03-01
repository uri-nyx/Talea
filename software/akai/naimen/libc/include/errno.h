#ifndef _ERRNO_H
#define _ERRNO_H

#ifndef _YVALS
#include <yvals.h>
#endif

#define EDOM   (_EDOM)
#define ERANGE (_ERANGE)
#define EFPOS  (_EFPOS)
#define _NERR  (_ERRMAX)

extern int errno;

#endif /* _ERRNO_H */
