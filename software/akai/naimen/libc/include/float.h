#ifndef _FLOAT_H
#define _FLOAT_H

/* Common Properties */
#define FLT_RADIX     2
#define FLT_ROUNDS    1    /* Round to nearest, even */

/* float - 32-bit IEEE 754 */
#define FLT_MANT_DIG  24
#define FLT_DIG       6
#define FLT_EPSILON   1.19209290E-07F
#define FLT_MIN_EXP   (-125)
#define FLT_MIN       1.17549435E-38F
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_EXP   128
#define FLT_MAX       3.40282347E+38F
#define FLT_MAX_10_EXP 38

/* double - identical to float on Sirius */
#define DBL_MANT_DIG  24
#define DBL_DIG       6
#define DBL_EPSILON   1.19209290E-07
#define DBL_MIN_EXP   (-125)
#define DBL_MIN       1.17549435E-38
#define DBL_MIN_10_EXP (-37)
#define DBL_MAX_EXP   128
#define DBL_MAX       3.40282347E+38
#define DBL_MAX_10_EXP 38

/* long double - identical to float on Sirius */
#define LDBL_MANT_DIG 24
#define LDBL_DIG      6
#define LDBL_EPSILON  1.19209290E-07L
#define LDBL_MIN_EXP  (-125)
#define LDBL_MIN      1.17549435E-38L
#define LDBL_MIN_10_EXP (-37)
#define LDBL_MAX_EXP  128
#define LDBL_MAX      3.40282347E+38L
#define LDBL_MAX_10_EXP 38

#endif /* _FLOAT_H */

