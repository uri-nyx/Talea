#ifndef _STDLIB_H
#define _STDLIB_H

#ifndef _YVALS
#include <yvals.h>
#endif

/* macros */
#ifndef NULL
#define NULL _NULL
#endif

#define EXIT_FAILURE _EXFAIL
#define EXIT_SUCCESS 0
#define RAND_MAX     32767
#define MB_CUR_MAX   _MBMAX /*TODO: properly implement locale */

/* types */

#ifndef _SIZET
#define _SIZET
typedef _Sizet size_t;
#endif

#ifndef _WCHART
#define _WCHART
typedef _Wchart wchar_t;
#endif

typedef struct {
    int quot; /* quotient */
    int rem;  /* remainder */
} div_t;

typedef struct {
    long quot; /* quotient */
    long rem;  /* remainder */
} ldiv_t;

typedef int _Cmpfun(const void *p, const void *q);

typedef struct {
    unsigned char  _State;
    unsigned short _Wchar;
} _Mbsave;

/* declarations */
void          abort(void);
int           abs(int j);
int           atexit(void (*func)(void));
double        atof(const char *nptr);
int           atoi(const char *nptr);
long          atol(const char *nptr);
void         *bsearch(const void *key, const void *base, size_t nmemb, size_t size, _Cmpfun comp);
void         *calloc(size_t nmemb, size_t size);
div_t         div(int numer, int denom);
void          exit(int status);
void          free(void *ptr);
char         *getenv(const char *name);
long          labs(long int j);
ldiv_t        ldiv(long int numer, long int denom);
void         *malloc(size_t size);
int           mblen(const char *s, size_t n);
size_t        mbstowcs(wchar_t *pwcs, const char *s, size_t n);
int           mbtowc(wchar_t *pwc, const char *s, size_t n);
void          qsort(void *base, size_t nmemb, size_t size, _Cmpfun comp);
int           rand(void);
void         *realloc(void *ptr, size_t size);
void          srand(unsigned int seed);
double        strtod(const char *nptr, char **endptr);
long          strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
int           system(const char *string);
int           wctomb(char *s, wchar_t wchar);
size_t        wcstombs(char *s, const wchar_t *pwcs, size_t n);

int           _Mbtowc(wchar_t *wchar, const char *s, size_t sz, _Mbsave *sav);
double        _Stod(const char *s, char **ss);
unsigned long _Stoul(const char *s, char **ss, int base);
int           _Wctomb(char *s, wchar_t wchar, char *t);

/* TODO: implement locale
extern char _Mbcurmax, _Wcxtowc;
extern _Mbsave _Mbxlen, _Mbxtowc;
*/

extern unsigned long _Randseed;

/* macro overrides */

#define atof(s)                  _Stod(s, 0)
#define atoi(s)                  (int)_Stoul(s, 0, 10)
#define atol(s)                  (long)_Stoul(s, 0, 10)
#define srand(seed)              (void)(_Randseed = (seed))
#define strtod(s, endptr)        _Stod(s, endptr);
#define strtoul(s, endptr, base) _Stoul(s, endptr, base);

/* TODO: implement locale
#define mblen(s, n) _Mbtowc(0, s, n, &_Mbxlen);
#define mbtowc(pwc, s, n) _Mbtowc(pwc, s, n, &_Mbxtowc)
#define wctomb(s, wchar) _Wctomb(s, wchar, &_Wcxtomb)
*/

#endif /* _STDLIB_H */
