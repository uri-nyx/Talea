#ifndef STDARG_H
#define STDARG_H

/* type definitions */
typedef unsigned char *va_list;

/* macros */
#define va_start(ap, A) ((va_list)(ap = ((va_list)(&A + 1))))
#define va_arg(ap, T)   (*(T *)((unsigned long)(ap += sizeof(T), ap - sizeof(T))))
#define va_end(ap)      (void)0

#endif /* STDARG_H */