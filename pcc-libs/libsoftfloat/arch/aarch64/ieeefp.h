/*	$  ieeefp.h,v 1.0 2020/06/03 09:26:10  $	*/
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

#ifndef _AARCH64_IEEEFP_H_
#define _AARCH64_IEEEFP_H_

typedef int fp_except;
#define FP_X_INV	8	/* invalid operation exception */
#define FP_X_DNML	15	/* denormalization exception */
#define FP_X_DZ		9	/* divide-by-zero exception */
#define FP_X_OFL	10	/* overflow exception */
#define FP_X_UFL	11	/* underflow exception */
#define FP_X_IMP	12	/* imprecise (loss of precision) */

typedef enum {
    FP_RN=(0 << 22),			/* round to nearest representable number */
    FP_RM=(1 << 22),			/* round toward negative infinity */
    FP_RP=(2 << 22),			/* round toward positive infinity */
    FP_RZ=(3 << 22)			/* round to zero (truncate) */
} fp_rnd;

#endif /* _AARCH64_IEEEFP_H_ */
