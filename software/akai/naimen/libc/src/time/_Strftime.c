/* _Strftime function */
#include "xtime.h"
#include <stdlib.h>
#include <string.h>

/* macros */
#define PUT(s, na)      \
    (void)(nput = (na), \
           0 < nput && (nchar += nput) <= bufsize ? (memcpy(buf, s, nput), buf += nput) : 0)

size_t _Strftime(char *buf, size_t bufsize, const char *fmt, const struct tm *t, _Tinfo *tin)
{
    const char *fmtsav, *s;
    size_t      len, lensav, nput;
    size_t      nchar = 0;

    for (s = fmt, len = strlen(fmt), fmtsav = NULL;; fmt = s) {
        /* scan for '%' */
        while (len > 0 && *s != '%') ++s, --len;

        /* copy literal text */
        if (fmt < s) PUT(fmt, s - fmt);

        if (len > 0) { /* found '%' */
            char        ac[20];
            int         m;
            const char *p;

            ++s; /* skip '%' */
            --len;

            p = _Gentime(t, tin, *s, &m, ac);

            ++s; /* skip format char */
            --len;

            if (m >= 0)
                PUT(p, m);
            else if (fmtsav == NULL) {
                fmtsav = s;
                s      = p;
                lensav = len;
                len    = (size_t)(-m);
            }
        }

        if (len == 0 && fmtsav == NULL) {
            PUT("", 1); /* null termination */
            return (nchar <= bufsize ? nchar - 1 : 0);
        } else if (len == 0) {
            s      = fmtsav;
            fmtsav = NULL;
            len    = lensav;
        }
    }
}
