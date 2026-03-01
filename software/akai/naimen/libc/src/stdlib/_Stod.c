#include "../math/xmath.h"
#include <ctype.h>
#include <limits.h>
//#include <locale.h>
#include <stdlib.h>

#define SIG_MAX 32


double _Stod(const char *s, char **endptr)
{
    const char  point = '.';//localeconv()->decimal_point[0]; //TODO: implement locale
    const char *sc;
    char        buf[SIG_MAX], sign;
    double      x;
    int         ndigit, nsig, nzero, olead, opoint;

    for (sc = s; isspace(*sc); ++sc);

    if (*sc == '-' || *sc == '+') {
        sign = *sc++;
    } else {
        sign = '+';
    }

    olead = -1, opoint = -1;

    for (ndigit = 0, nsig = 0, nzero = 0;; ++sc) {
        if (*sc == point) {
            if (0 <= opoint) {
                break;
            } else {
                opoint = ndigit;
            }
        } else if (*sc == '0') {
            ++nzero, ++ndigit;
        } else if (!isdigit(*sc)) {
            break;
        } else {
            if (olead < 0) {
                olead = nzero, nzero = 0, ndigit = 0;
            } else {
                for (; 0 < nzero && nsig < SIG_MAX; --nzero) buf[nsig++] = 0;
            }

            ++ndigit;

            if (nsig < SIG_MAX) buf[nsig++] = *sc - '0';
        }
    }

    if (ndigit == 0) {
        if (endptr) *endptr = (char *)s;
        return 0.0;
    }

    for (; 0 < nsig && buf[nsig - 1] == 0; --nsig);

    /* compute significand */
    {
        const char   *pc = buf;
        int           n;
        long          lo[(SIG_MAX / 8) + 1];
        long         *pl    = &lo[nsig >> 3];
        static double fac[] = { 0, 1e8, 1e16, 1e24, 1e32 };

        for (*pl = 0, n = nsig; 0 < n; --n) {
            if ((n & 0x7) == 0) {
                *--pl = *pc++;
            } else {
                *pl = *pl * 10 + *pc++;
            }
        }

        for (x = (double)lo[0], n = 0; ++n <= (nsig >> 3);) {
            if (lo[n] != 0) {
                x += fac[n] * (double)lo[n];
            }
        }
    }

    /* fold exponent */
    {
        long  lexp = 0;
        short sexp;

        if (*sc == 'e' || *sc == 'E') {
            const char *scsav = sc;
            char        esign;

            ++sc;
            if (*sc == '+' || *sc == '-') {
                esign = *sc++;
            } else {
                esign = '+';
            }

            if (!isdigit(*sc)) {
                sc = scsav;
            } else {
                for (; isdigit(*sc); ++sc) {
                    if (lexp < 100000) {
                        lexp = lexp * 10 + *sc - '0';
                    }
                }

                if (esign == '-') lexp = -lexp;
            }
        }
        if (endptr) *endptr = (char *)sc;
        if (opoint < 0) {
            lexp += ndigit - nsig;
        } else {
            lexp += opoint - olead - nsig;
        }

        sexp = lexp < SHRT_MIN ? SHRT_MIN : lexp < SHRT_MAX ? (short)lexp : SHRT_MAX;
        x    = _Dtento(x, sexp);
        return sign == '-' ? -x : x;
    }
}
