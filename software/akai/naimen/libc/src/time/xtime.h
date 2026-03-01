/* xtime.h internal header */
#include <time.h>
/* macros */
#define WDAY 1 /* to get day of week right */
               /* type definitions */
typedef struct {
    unsigned char wday, hour, day, mon, year;
} Dstrule;

/* type definitions */
typedef struct {
    const char *_Ampm;
    const char *_Days;
    const char *_Formats;
    const char *_Isdst;
    const char *_Months;
    const char *_Tzone;
} _Tinfo;
/* declarations */
extern _Tinfo _Times;

/* internal declarations */
const char *_Gentime(const struct tm *, _Tinfo *, char, int *, char *);
const char *_Gettime(const char *, int, int *);
int         _Isdst(const struct tm *);
size_t      _Strftime(char *, size_t, const char *, const struct tm *, _Tinfo *);
