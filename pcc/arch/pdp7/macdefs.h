/*	$Id: macdefs.h,v 1.11 2023/08/20 15:30:30 ragge Exp $	*/
/*
 * Copyright (c) 2003 Anders Magnusson (ragge@ludd.luth.se).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Machine-dependent defines for both passes.
 */
/*
 * Convert (multi-)character constant to integer.
 */
#define makecc(val,i)	lastcon = val;

#define ARGINIT		18	/* # bits above fp where arguments start */
#define AUTOINIT	0	/* # bits below fp where automatics start */

/*
 * Storage space requirements
 */
#define SZCHAR		9
#define SZBOOL		9
#define SZSHORT		18
#define SZINT		18
#define SZFLOAT		18
#define SZDOUBLE	36
#define SZLDOUBLE	36
#define SZLONG		36
#define SZLONGLONG	36	/* XXX */
#define SZPOINT(t)	18	/* XXX */

/*
 * Alignment constraints
 */
#define ALCHAR		9
#define ALBOOL		9
#define ALSHORT		18
#define ALINT		18
#define ALFLOAT		18
#define ALDOUBLE	18
#define ALLDOUBLE	18
#define ALLONG		18
#define ALLONGLONG	18
#define ALPOINT		18
#undef ALSTRUCT		/* Not defined if ELF ABI */
#define ALSTACK		18 
#define	ALMAX		18	/* not yet supported type */

/*
 * Min/max values.
 */
#define	MIN_CHAR	-256
#define	MAX_CHAR	255
#define	MAX_UCHAR	511
#define	MIN_SHORT	-131072
#define	MAX_SHORT	131071
#define	MAX_USHORT	262144
#define	MIN_INT		MIN_SHORT
#define	MAX_INT		MAX_SHORT
#define	MAX_UNSIGNED	MAX_USHORT
#define	MIN_LONG	MIN_INT
#define	MAX_LONG	MAX_INT
#define	MAX_ULONG	MAX_UNSIGNED
#define	MIN_LONGLONG	MIN_LONG
#define	MAX_LONGLONG	MAX_LONG
#define	MAX_ULONGLONG	MAX_ULONG

/* Default char is signed */
#define	CHAR_UNSIGNED
#define	BOOL_TYPE	UCHAR	/* what used to store _Bool */
#undef UNALIGNED_ACCESS
/*
 * Use large-enough types.
 */
typedef	long long CONSZ;
typedef	unsigned long long U_CONSZ;
typedef long long OFFSZ;

#define CONFMT	"%llo"		/* format for printing constants */
#define LABFMT	"L%d"		/* format for printing labels */

#define STACK_DOWN 		/* stack grows negatively for automatics */

#undef	FIELDOPS		/* no bit-field instructions */
#define TARGET_ENDIAN TARGET_LE

#define FINDMOPS	/* to find isz */
#define	CC_DIV_0	/* division by zero is safe in the compiler */
#define BYTEOFF(x)	((x)&01)
#define	MYALIGN
#define MYINSTRING
#define MYDOTFILE
#define	printdotfile(ftitle)	/* just ignore */
void myendinit(void);
#define	MYENDINIT	myendinit();
#define	WORD_ADDRESSED

#define	szty(t)	(((t) == DOUBLE || (t) == FLOAT || \
	(t) == LONGLONG || (t) == ULONGLONG) ? 2 : (t) == LDOUBLE ? 3 : 1)

/*
 * pdp7 has only one register.
 * We emulate 7 memory positions to make the compiler happier.
 */
#define	AC	000	/* Scratch and return register */
#define	POS1	001
#define	POS2	002
#define	POS3	003
#define	POS4	004
#define	POS5	005
#define	POS6	006
#define	POS7	007

#define	FP	010
#define	SP	011

#define	MAXREGS	012

#define	RSTATUS	SAREG|TEMPREG, SBREG|TEMPREG, SBREG|TEMPREG, SBREG|TEMPREG, \
		SBREG|TEMPREG, SBREG|TEMPREG, SBREG|TEMPREG, SBREG|TEMPREG, \
		0, 0

#define	ROVERLAP { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, { -1 }, \
		{ -1 }, { -1 }, { -1 },


/* Return a register class based on the type of the node */
#define PCLASS(p) (p->n_op == REG && regno(p) > 7 ? 0 : \
	(p->n_op == REG && regno(p)) ? SBREG : SAREG)

#define	NUMCLASS 	2	/* highest number of reg classes used */

int COLORMAP(int c, int *r);
#define	GCLASS(x)	((x) == 0 ? CLASSA : CLASSB)
#define DECRA(x,y)	(((x) >> (y*6)) & 63)	/* decode encoded regs */
#define	ENCRD(x)	(x)		/* Encode dest reg in n_reg */
#define ENCRA1(x)	((x) << 6)	/* A1 */
#define ENCRA2(x)	((x) << 12)	/* A2 */
#define ENCRA(x,y)	((x) << (6+y*6))	/* encode regs in int */
#define	RETREG(x)	AC

/* XXX - to die */
#define FPREG	FP	/* frame pointer */
#define STKREG	SP	/* stack pointer */

#define	SLDFPSP		(MAXSPECIAL+1)	/* load fp or sp */

extern int msettings;
#define	M_CHAR18	001	/* Use chars that are word size */

#define	ISCHAR18	(msettings & M_CHAR18)
#define	ISCHAR9		((msettings & M_CHAR18) == 0)

/* For acceptable() */
#define	USECHAR9	0200
#define	USECHAR18	0400

/* floating point definitions XXX incorrect here */
#define FDFLOAT
#define DEFAULT_FPI_DEFS { &fpi_ffloat, &fpi_dfloat, &fpi_dfloat }

