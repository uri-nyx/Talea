/* _Dtento function -- IEEE 754 version */
#include <errno.h>

/* From Plauger */

#include "xmath.h"

/* macros */
#define NBITS (48 + _DOFF)
#if _D0
#define INIT(w0) 0, 0, 0, w0
#else
#define INIT(w0) w0, 0, 0, 0
#endif
/* static data */
_Dconst _Hugeval = { INIT(_DMAX << _DOFF) };
_Dconst _Inf     = { INIT(_DMAX << _DOFF) };
_Dconst _Nan     = { INIT(_DNAN) };
_Dconst _Rteps   = { INIT((_DBIAS - NBITS / 2) << _DOFF) };
_Dconst _Xbig    = { INIT((_DBIAS + NBITS / 2) << _DOFF) };

#define _AD0    0          /* Most significant word */
#define _AD1    1          /* Least significant word */
#define _ADMASK 0x7f800000 /* Exponent mask for 32-bit float */
#define _ADMAX  0xff       /* Max exponent (255) */
#define _ADOFF  23         /* Fraction bits offset */
#define _ADFRAC 0x007fffff /* Fraction mask */
#define _ADBIAS 0x7F       /* Exponent bias (127) */
#define _ADSIGN 0x80000000 /* Sign bit */

short _Dscale(double *px, short xexp)
{
    unsigned long *ps  = (unsigned long *)px; /* reinterpret as 32‑bit float */
    unsigned long  w   = ps[_AD0];
    short          exp = (w & _ADMASK) >> _ADOFF;

    /* NaN or INF */
    if (exp == _ADMAX) {
        return (w & _ADFRAC) ? NAN : INF;
    }

    /* Zero */
    if ((w & ~_ADSIGN) == 0) {
        return 0;
    }

    /* Subnormal → normalize */
    if (exp == 0) {
        short norm = _DDnorm(ps);
        if (norm <= 0) {
            /* became zero */
            return 0;
        }
        w   = ps[_AD0];
        exp = (w & _ADMASK) >> _ADOFF;
    }

    /* Apply exponent change */
    {
        long newexp = (long)exp + xexp;

        /* Overflow → ±INF */
        if (newexp >= _ADMAX) {
            ps[_AD0] = (w & _ADSIGN) | (_ADMAX << _ADOFF);
            return INF;
        }

        /* Underflow → ±0 */
        if (newexp <= 0) {
            ps[_AD0] = (w & _ADSIGN); /* keep sign, zero everything else */
            return 0;
        }

        /* Normal result */
        ps[_AD0] = (w & _ADSIGN) | ((unsigned long)newexp << _ADOFF) | (w & _ADFRAC);
        return FINITE;
    }
}

static short _DDnorm(unsigned long *ps)
{ /* normalize double fraction */
    short xchar = 0;

    if (ps[_AD0] << 1 == 0 && ps[_AD1] == 0) return (0); /* zero */

    /* normalize by shifting left until leading bit in fraction */
    while ((ps[_AD0] & 0x7f800000) == 0) { /* shift left by 1 */
        ps[_AD0] = ps[_AD0] << 1 | (ps[_AD1] >> 31);
        ps[_AD1] <<= 1;
        --xchar;
    }
    return (xchar);
}

short _Ldunscale(short *pex, long double *px)
{
    unsigned long *ps  = (unsigned long *)px; /* 32‑bit float layout */
    unsigned long  w   = ps[_AD0];
    short          exp = (w & _ADMASK) >> _ADOFF; /* NaN or INF */
    if (exp == _ADMAX) {
        *pex = 0;
        return (w & _ADFRAC) ? NAN : INF;
    } /* Zero */
    if ((w & ~_ADSIGN) == 0) {
        *pex = 0;
        return 0;
    } /* Finite, normalize to [1/2, 1) */
    if (exp == 0) {
        /* subnormal: you can either normalize properly or just treat as zero for now */
        /* simplest safe behavior: treat all subnormals as zero */
        *pex = 0;
        return 0;
    } /* Clear exponent to bias, preserve sign */
    ps[_AD0] = (w & _ADSIGN) | ((unsigned long)_ADBIAS << _ADOFF) | (w & _ADFRAC);
    *pex     = exp - _ADBIAS;
    return FINITE;
}

static const float pow10_pos[] = { 1e1f, 1e2f, 1e4f, 1e8f, 1e16f, 1e32f };
static const float pow10_neg[] = { 1e-1f, 1e-2f, 1e-4f, 1e-8f, 1e-16f, 1e-32f };

double _Dtento(double x, short n)
{
    if (n == 0 || x == 0.0f) return x; /* scale up */
    if (n > 0) {
        unsigned int u = (unsigned int)n;
        int          i = 0;
        while (u && i < 6) {
            if (u & 1) x *= pow10_pos[i];
            u >>= 1;
            i++;
        }
        return x;
    } /* scale down */
    {
        unsigned int u = (unsigned int)(-n);
        int          i = 0;
        while (u && i < 6) {
            if (u & 1) x *= pow10_neg[i];
            u >>= 1;
            i++;
        }
        return x;
    }
}
