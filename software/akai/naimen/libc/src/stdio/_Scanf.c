/* _Scanf function — ASCII‑only version */
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "xstdio.h"

int _Scanf(int (*pfn)(void *, int), void *arg, const char *fmt, va_list ap)
{
    int  nconv = 0;
    _Sft x;

    x.pfn   = pfn;
    x.arg   = arg;
    x.ap    = ap;
    x.nchar = 0;

    for (x.s = fmt;; ++x.s) { /* parse format string */
        int ch;

        /* match any literal or white-space */
        while (*x.s && *x.s != '%') {
            if (isspace((unsigned char)*x.s)) {
                /* skip whitespace in format */
                while (isspace((unsigned char)*x.s)) ++x.s;

                /* skip whitespace in input */
                while (isspace(ch = GET(&x)));
                UNGETN(&x, ch);
            } else {
                /* literal match */
                ch = GET(&x);
                if (ch != (unsigned char)*x.s) {
                    UNGETN(&x, ch);
                    return nconv; /* mismatch ends scanning */
                }
                ++x.s;
            }
        }

        if (*x.s == '\0') return nconv;

        /* process a conversion specifier */
        if (*x.s == '%') {
            int code;

            ++x.s; /* skip '%' */

            x.noconv = *x.s == '*' ? *x.s++ : '\0';

            for (x.width = 0; isdigit((unsigned char)*x.s); ++x.s)
                if (x.width < _WMAX) x.width = x.width * 10 + *x.s - '0';

            x.qual = strchr("hlL", *x.s) ? *x.s++ : '\0';

            if (!strchr("cn[", *x.s)) {
                /* match leading white-space in input */
                while (isspace(ch = GET(&x)));
                UNGETN(&x, ch);
            }

            if ((code = _Getfld(&x)) <= 0) return (nconv == 0 ? code : nconv);

            if (x.stored) ++nconv;

            /* DO NOT increment x.s here — the for-loop does it */
        }
    }
}
