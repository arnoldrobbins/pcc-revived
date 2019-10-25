#ifndef _LIBPCC_FLOAT_H_
#define _LIBPCC_FLOAT_H_

/*
 * number of decimal digits needed to represent all the
 * significant digits for all internal floating-point formats
 */
#define DECIMAL_DIG 21

/*
 * the floating-point expression evaluation method:
 *	-1	indeterminate
 *	0	evaluate to range and precision of type
 *	1	evaluate to range and precision of double type
 *	2	evaluate to range and precision of long double type
 */
#ifdef __FLT_EVAL_METHOD__
#define FLT_EVAL_METHOD __FLT_EVAL_METHOD__
#else
#define FLT_EVAL_METHOD 0
#endif

#define FLT_RADIX 2

#define FLT_HAS_SUBNORM	__FLT_HAS_SUBNORM__
#define FLT_MANT_DIG	__FLT_MANT_DIG__
#define FLT_DIG		__FLT_DIG__
#define FLT_ROUNDS	__FLT_ROUNDS__
#define FLT_EPSILON	__FLT_EPSILON__
#define FLT_MIN_EXP	__FLT_MIN_EXP__
#define FLT_MIN		__FLT_MIN__
#define FLT_MIN_10_EXP	__FLT_MIN_10_EXP__
#define FLT_MAX_EXP	__FLT_MAX_EXP__
#define FLT_MAX		__FLT_MAX__
#define FLT_MAX_10_EXP	__FLT_MAX_10_EXP__
#define FLT_TRUE_MIN	__FLT_TRUE_MIN__

#define DBL_HAS_SUBNORM	__DBL_HAS_SUBNORM__
#define DBL_MANT_DIG	__DBL_MANT_DIG__
#define DBL_DIG		__DBL_DIG__
#define DBL_ROUNDS	__DBL_ROUNDS__
#define DBL_EPSILON	__DBL_EPSILON__
#define DBL_MIN_EXP	__DBL_MIN_EXP__
#define DBL_MIN		__DBL_MIN__
#define DBL_MIN_10_EXP	__DBL_MIN_10_EXP__
#define DBL_MAX_EXP	__DBL_MAX_EXP__
#define DBL_MAX		__DBL_MAX__
#define DBL_MAX_10_EXP	__DBL_MAX_10_EXP__
#define DBL_TRUE_MIN	__DBL_TRUE_MIN__

#define LDBL_HAS_SUBNORM	__LDBL_HAS_SUBNORM__
#define LDBL_MANT_DIG	__LDBL_MANT_DIG__
#define LDBL_DIG	__LDBL_DIG__
#define LDBL_ROUNDS	__LDBL_ROUNDS__
#define LDBL_EPSILON	__LDBL_EPSILON__
#define LDBL_MIN_EXP	__LDBL_MIN_EXP__
#define LDBL_MIN	__LDBL_MIN__
#define LDBL_MIN_10_EXP	__LDBL_MIN_10_EXP__
#define LDBL_MAX_EXP	__LDBL_MAX_EXP__
#define LDBL_MAX	__LDBL_MAX__
#define LDBL_MAX_10_EXP	__LDBL_MAX_10_EXP__
#define LDBL_TRUE_MIN	__LDBL_TRUE_MIN__


#endif
