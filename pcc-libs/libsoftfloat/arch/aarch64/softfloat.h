/*	$  softfloat.h,v 1.0 2020/06/03 09:55:51  $	*/
/*
 * Copyright (c) 2020 Puresoftware Ltd.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
-------------------------------------------------------------------------------
The macro `FLOATX80' must be defined to enable the extended double-precision
floating-point format `floatx80'.  If this macro is not defined, the
`floatx80' type will not be defined, and none of the functions that either
input or output the `floatx80' type will be defined.  The same applies to
the `FLOAT128' macro and the quadruple-precision format `float128'.
-------------------------------------------------------------------------------
*/
/* #define FLOATX80 */
/* #define FLOAT128 */

#include <ieeefp.h>

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point types.
-------------------------------------------------------------------------------
*/
typedef unsigned int float32;
typedef unsigned long long float64;
#ifdef FLOATX80
typedef struct {
    unsigned short high;
    unsigned long long low;
} floatx80;
#endif
#ifdef FLOAT128
typedef struct {
    unsigned long long high, low;
} float128;
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point underflow tininess-detection mode.
-------------------------------------------------------------------------------
*/
#ifndef SOFTFLOAT_FOR_GCC
extern int float_detect_tininess;
#endif
enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point rounding mode.
-------------------------------------------------------------------------------
*/
extern fp_rnd float_rounding_mode;
enum {
    float_round_nearest_even = FP_RN,
    float_round_to_zero      = FP_RZ,
    float_round_down         = FP_RM,
    float_round_up           = FP_RP
};

/*
-------------------------------------------------------------------------------
Software IEC/IEEE floating-point exception flags.
-------------------------------------------------------------------------------
*/
extern fp_except float_exception_flags;
extern fp_except float_exception_mask;
enum {
    float_flag_inexact   = FP_X_IMP,
    float_flag_underflow = FP_X_UFL,
    float_flag_overflow  = FP_X_OFL,
    float_flag_divbyzero = FP_X_DZ,
    float_flag_invalid   = FP_X_INV
};

/*
-------------------------------------------------------------------------------
Routine to raise any or all of the software IEC/IEEE floating-point
exception flags.
-------------------------------------------------------------------------------
*/
void float_raise( fp_except );

/*
-------------------------------------------------------------------------------
Software IEC/IEEE integer-to-floating-point conversion routines.
-------------------------------------------------------------------------------
*/
float32 int32_to_float32( int );
float64 int32_to_float64( int );
#ifdef FLOATX80
floatx80 int32_to_floatx80( int );
#endif
#ifdef FLOAT128
float128 int32_to_float128( int );
#endif
#ifndef SOFTFLOAT_FOR_GCC /* __floatdi?f is in libgcc2.c */
float32 int64_to_float32( long long );
float64 int64_to_float64( long long );
#ifdef FLOATX80
floatx80 int64_to_floatx80( long long );
#endif
#ifdef FLOAT128
float128 int64_to_float128( long long );
#endif
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE single-precision conversion routines.
-------------------------------------------------------------------------------
*/
int float32_to_int32( float32 );
int float32_to_int32_round_to_zero( float32 );
#if defined(SOFTFLOAT_FOR_GCC) && defined(SOFTFLOAT_NEED_FIXUNS)
unsigned int float32_to_uint32_round_to_zero( float32 );
#endif
#ifndef SOFTFLOAT_FOR_GCC /* __fix?fdi provided by libgcc2.c */
long long float32_to_int64( float32 );
long long float32_to_int64_round_to_zero( float32 );
#endif
float64 float32_to_float64( float32 );
#ifdef FLOATX80
floatx80 float32_to_floatx80( float32 );
#endif
#ifdef FLOAT128
float128 float32_to_float128( float32 );
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE single-precision operations.
-------------------------------------------------------------------------------
*/
float32 float32_round_to_int( float32 );
float32 float32_add( float32, float32 );
float32 float32_sub( float32, float32 );
float32 float32_mul( float32, float32 );
float32 float32_div( float32, float32 );
float32 float32_rem( float32, float32 );
float32 float32_sqrt( float32 );
int float32_eq( float32, float32 );
int float32_le( float32, float32 );
int float32_lt( float32, float32 );
int float32_eq_signaling( float32, float32 );
int float32_le_quiet( float32, float32 );
int float32_lt_quiet( float32, float32 );
#ifndef SOFTFLOAT_FOR_GCC
int float32_is_signaling_nan( float32 );
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE double-precision conversion routines.
-------------------------------------------------------------------------------
*/
int float64_to_int32( float64 );
int float64_to_int32_round_to_zero( float64 );
#if defined(SOFTFLOAT_FOR_GCC) && defined(SOFTFLOAT_NEED_FIXUNS)
unsigned int float64_to_uint32_round_to_zero( float64 );
#endif
#ifndef SOFTFLOAT_FOR_GCC /* __fix?fdi provided by libgcc2.c */
long long float64_to_int64( float64 );
long long float64_to_int64_round_to_zero( float64 );
#endif
float32 float64_to_float32( float64 );
#ifdef FLOATX80
floatx80 float64_to_floatx80( float64 );
#endif
#ifdef FLOAT128
float128 float64_to_float128( float64 );
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE double-precision operations.
-------------------------------------------------------------------------------
*/
float64 float64_round_to_int( float64 );
float64 float64_add( float64, float64 );
float64 float64_sub( float64, float64 );
float64 float64_mul( float64, float64 );
float64 float64_div( float64, float64 );
float64 float64_rem( float64, float64 );
float64 float64_sqrt( float64 );
int float64_eq( float64, float64 );
int float64_le( float64, float64 );
int float64_lt( float64, float64 );
int float64_eq_signaling( float64, float64 );
int float64_le_quiet( float64, float64 );
int float64_lt_quiet( float64, float64 );
#ifndef SOFTFLOAT_FOR_GCC
int float64_is_signaling_nan( float64 );
#endif

#ifdef FLOATX80

/*
-------------------------------------------------------------------------------
Software IEC/IEEE extended double-precision conversion routines.
-------------------------------------------------------------------------------
*/
int floatx80_to_int32( floatx80 );
int floatx80_to_int32_round_to_zero( floatx80 );
long long floatx80_to_int64( floatx80 );
long long floatx80_to_int64_round_to_zero( floatx80 );
float32 floatx80_to_float32( floatx80 );
float64 floatx80_to_float64( floatx80 );
#ifdef FLOAT128
float128 floatx80_to_float128( floatx80 );
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE extended double-precision rounding precision.  Valid
values are 32, 64, and 80.
-------------------------------------------------------------------------------
*/
extern int floatx80_rounding_precision;

/*
-------------------------------------------------------------------------------
Software IEC/IEEE extended double-precision operations.
-------------------------------------------------------------------------------
*/
floatx80 floatx80_round_to_int( floatx80 );
floatx80 floatx80_add( floatx80, floatx80 );
floatx80 floatx80_sub( floatx80, floatx80 );
floatx80 floatx80_mul( floatx80, floatx80 );
floatx80 floatx80_div( floatx80, floatx80 );
floatx80 floatx80_rem( floatx80, floatx80 );
floatx80 floatx80_sqrt( floatx80 );
int floatx80_eq( floatx80, floatx80 );
int floatx80_le( floatx80, floatx80 );
int floatx80_lt( floatx80, floatx80 );
int floatx80_eq_signaling( floatx80, floatx80 );
int floatx80_le_quiet( floatx80, floatx80 );
int floatx80_lt_quiet( floatx80, floatx80 );
int floatx80_is_signaling_nan( floatx80 );

#endif

#ifdef FLOAT128

/*
-------------------------------------------------------------------------------
Software IEC/IEEE quadruple-precision conversion routines.
-------------------------------------------------------------------------------
*/
int float128_to_int32( float128 );
int float128_to_int32_round_to_zero( float128 );
long long float128_to_int64( float128 );
long long float128_to_int64_round_to_zero( float128 );
float32 float128_to_float32( float128 );
float64 float128_to_float64( float128 );
#ifdef FLOATX80
floatx80 float128_to_floatx80( float128 );
#endif

/*
-------------------------------------------------------------------------------
Software IEC/IEEE quadruple-precision operations.
-------------------------------------------------------------------------------
*/
float128 float128_round_to_int( float128 );
float128 float128_add( float128, float128 );
float128 float128_sub( float128, float128 );
float128 float128_mul( float128, float128 );
float128 float128_div( float128, float128 );
float128 float128_rem( float128, float128 );
float128 float128_sqrt( float128 );
int float128_eq( float128, float128 );
int float128_le( float128, float128 );
int float128_lt( float128, float128 );
int float128_eq_signaling( float128, float128 );
int float128_le_quiet( float128, float128 );
int float128_lt_quiet( float128, float128 );
int float128_is_signaling_nan( float128 );

#endif

