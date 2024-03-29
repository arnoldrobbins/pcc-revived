/*	$Id: local2.c,v 1.34 2023/08/20 14:38:27 ragge Exp $	*/
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

#include "pass2.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#if defined(MACHOABI)
#define EXPREFIX	"_"
#else
#define EXPREFIX	""
#endif

#define LOWREG		0
#define HIREG		1

char *rnames[] = {
	REGPREFIX "r0", REGPREFIX "r1",
	REGPREFIX "r2", REGPREFIX "r3",
	REGPREFIX "r4", REGPREFIX "r5",
	REGPREFIX "r6", REGPREFIX "r7",
	REGPREFIX "r8", REGPREFIX "r9",
	REGPREFIX "r10", REGPREFIX "r11",
	REGPREFIX "r12", REGPREFIX "r13",
	REGPREFIX "r14", REGPREFIX "r15",
	REGPREFIX "r16", REGPREFIX "r17",
	REGPREFIX "r18", REGPREFIX "r19",
	REGPREFIX "r20", REGPREFIX "r21",
	REGPREFIX "r22", REGPREFIX "r23",
	REGPREFIX "r24", REGPREFIX "r25",
	REGPREFIX "r26", REGPREFIX "r27",
	REGPREFIX "r28", REGPREFIX "r29",
	REGPREFIX "r30", REGPREFIX "r31",
	"r4\0r3\0", "r5\0r4\0", "r6\0r5\0", "r7\0r6\0",
	"r8\0r7\0", "r9\0r8\0", "r10r9\0", "r15r14", "r17r16",
	"r19r18", "r21r20", "r23r22", "r25r24", "r27r26",
	"r29r28", "r31r30",
	REGPREFIX "f0", REGPREFIX "f1",
	REGPREFIX "f2", REGPREFIX "f3",
	REGPREFIX "f4", REGPREFIX "f5",
	REGPREFIX "f6", REGPREFIX "f7",
	REGPREFIX "f8", REGPREFIX "f9",
	REGPREFIX "f10", REGPREFIX "f11",
	REGPREFIX "f12", REGPREFIX "f13",
	REGPREFIX "f14", REGPREFIX "f15",
	REGPREFIX "f16", REGPREFIX "f17",
	REGPREFIX "f18", REGPREFIX "f19",
	REGPREFIX "f20", REGPREFIX "f21",
	REGPREFIX "f22", REGPREFIX "f23",
	REGPREFIX "f24", REGPREFIX "f25",
	REGPREFIX "f26", REGPREFIX "f27",
	REGPREFIX "f28", REGPREFIX "f29",
	REGPREFIX "f30", REGPREFIX "f31",
};


typedef struct n_regs {
	int gpr;
	int fpr;
} n_regs;


static int argsize(NODE *p, n_regs *nr);

static int p2calls;
static int p2temps;		/* TEMPs which aren't autos yet */
static int p2framesize;
static int p2maxstacksize;

void
deflab(int label)
{
	printf(LABFMT ":\n", label);
}

static TWORD ftype;

/*
 * Print out the prolog assembler.
 */
void
prologue(struct interpass_prolog *ipp)
{
int i, sz, addto = p2framesize; //p2maxautooff;

#ifdef PCC_DEBUG
	if (x2debug)
		printf("prologue: type=%d, lineno=%d, name=%s, vis=%d, ipptype=%d, autos=%d, tmpnum=%d, lblnum=%d\n",
			ipp->ipp_ip.type,
			ipp->ipp_ip.lineno,
			ipp->ipp_name,
			ipp->ipp_flags & IF_VISIBLE,
			ipp->ipp_type,
			ipp->ipp_autos,
			ipp->ip_tmpnum,
			ipp->ip_lblnum);

	printf("; p2framesize = %d, p2calls = %d, p2autooff = %d\n", 
			p2framesize, p2calls, p2autooff);
#endif

	if (p2framesize == 0)
		return; /* no need to create a stack frame */
				
#if defined(ELFABI)

	sz = p2autooff;

#ifdef FPREG
	printf("\tmr %s,%s	; preserve FPREG\n", rnames[R0], rnames[FPREG]);
	printf("\tmr %s,%s	; establish frame pointer\n", rnames[FPREG], rnames[R1]);
#else
	printf("\tmr %s,%s	; preserve previous stack pointer\n", rnames[R0], rnames[R1]);
#endif

	/* create the new stack frame */
	if (addto > 32767) {
		printf("\tlis %s,%d	; stack frame ? 32767\n", rnames[R0], (-addto) >> 16);
		printf("\tori %s,%s,%d\n", rnames[R0],
		    rnames[R0], (-addto) & 0xffff);
		printf("\tstwux %s,%s,%s	; move the stack pointer\n", rnames[SPREG],
		    rnames[R1], rnames[R0]);
	} else {
		printf("\tstwu %s,-%d(%s)	; move the stack pointer\n", rnames[R1], addto, rnames[SPREG]);
	}

	if (p2calls != 0 || kflag) {
		/* save the previous stack location */
#ifdef FPREG
		printf("\tstw %s,-4(%s)	; save FPREG relative to frame pointer\n", rnames[R0], rnames[FPREG]);
		printf("\tstw %s,0(%s)	; save previous stack pointer\n", rnames[FPREG], rnames[SPREG]);
#else
		printf("\tstw %s,0(%s)	; save previous stack pointer\n", rnames[R0], rnames[SPREG]);
#endif
		/* get return address (not required for leaf function) */
		printf("\tmflr %s\n", rnames[R0]);
		printf("\tstw %s,4(%s)\n", rnames[R0], rnames[R1]);
		
		if (kflag) {
		/* save registers R30 and R31 */
			printf("\tstmw %s,-8(%s)	; save GOTREG\n", rnames[GOTREG], rnames[FPREG]);
			printf("\tbl _GLOBAL_OFFSET_TABLE_@local-4\n");
			printf("\tmflr %s\n", rnames[GOTREG]);
		}
	}

	
#elif defined(MACHOABI)
	/* 
	 * Mach-O defines a "linkage area" where the LR, CR and SP are saved.
	 * This area is used by the CALLEE but MUST be included in the stack
	 * area of the CALLER.  Additionally, values are stored in slightly
	 * different locations, so it is easier to craft a different prologue.
	 *
	 * The layout:
	 *
	 *		orig sp = r1
	 * 		(orig sp)+8: LR
	 *		(orig sp)+4: CR
	 *		orig sp => new fp (in r30)
	 *		(new fp)-4 = orig PICREG (= orig r31)
	 *		(new fp)-8 = orig fp (= orig r30)
	 *		... (save other registers below FPREG)
	 *		r1	= new sp
	 *
	 */
	char* scratch;
	sz = 4;
	
	if (p2calls) {
	 
		printf("\tmflr  %s			; save the link register\n", rnames[R0]);
		printf("\tstw   %s,8(%s)\n", rnames[R0],rnames[SPREG]);
		printf("\tmfcr  %s			; save the condition register\n", rnames[R0]);
		printf("\tstw   %s,4(%s)\n", rnames[R0],rnames[SPREG]);
#if 0 //defined(FPREG)	 
		/* Mach-O does not use a frame pointer, but pcc does. */
		printf("\tmr %s,%s	; preserve FPREG\n", rnames[R0], rnames[FPREG]);
		printf("\tmr %s,%s	; establish new frame pointer\n", rnames[FPREG], rnames[SPREG]);
#endif

		
#if 0 // defined(FPREG)
		printf("\tstw %s,-4(%s)		; save PICREG relative to frame pointer\n", rnames[PICREG], rnames[FPREG]);
		printf("\tstw %s,-8(%s)		; save FPREG relative to frame pointer\n", rnames[R0], rnames[FPREG]);
		scratch = rnames[FPREG];
#else
		printf("\tstw %s,-4(%s)		; save PICREG relative to stack pointer\n", rnames[PICREG], rnames[SPREG]);
		scratch = rnames[SPREG];
#endif

	} else {
#if 1
		/* use the "Red area" to save volatiles */
		printf("\tmr %s, %s\n", rnames[R2], rnames[R31]);
		printf("\tmflr %s\n", rnames[R0]);
		scratch = rnames[SPREG];
#endif
	}

	/* save non-volatile registers from the frame/stack pointer down */	
	/* stmw is an option for saving off multiple registers but slower than stw */
printf("; p2env.p_regs = 0x%x%x\n", p2env.p_regs[0], p2env.p_regs[1]);
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
			sz += (SZLONG/SZCHAR);
			if (sz == addto)
				cerror("collision between stack and frame");
		
			printf("\tstw %s,-%d(%s)\n", rnames[i], sz, scratch);
		}
	}
	
	if (p2calls) {
		/* create the new stack frame */
		if (addto > 32767) {
			printf("\tlis %s,%d	; stack frame ? 32767\n", rnames[R0], (-addto) >> 16);
			printf("\tori %s,%s,%d\n", rnames[R0],
			    rnames[R0], (-addto) & 0xffff);
			printf("\tstwux %s,%s,%s	; move the stack pointer\n", rnames[SPREG],
			    rnames[R1], rnames[R0]);
		} else {
			printf("\tstwu %s,-%d(%s)	; move the stack pointer\n", rnames[SPREG], addto, rnames[SPREG]);
		}
	}
	
	printf("\tbcl 20,31,\"L%s$pb\"\n", ipp->ipp_name + 1);
	printf("\"L%s$pb\":\n", ipp->ipp_name + 1);
	printf("\tmflr %s\n", rnames[PICREG]);

	if (!p2calls) {
		printf("\tmtlr %s\n", rnames[R0]);
		printf("\tmr %s, %s\n", rnames[R0], rnames[R2]);
	}

#elif defined(AOUTABI)



#else
#error unsupported ABI in local2.c:prologue()

#endif
}


void
eoftn(struct interpass_prolog *ipp)
{

int i, sz, addto = p2framesize;
#if defined(ELFABI)
int idx = 0;
#elif defined(MACHOABI)
int idx = 1;
#endif

#ifdef PCC_DEBUG
	if (x2debug)
		printf("eoftn:\n");
#endif

	if (ipp->ipp_ip.ip_lbl == 0)
		return; /* no code needs to be generated */


	if (p2framesize == 0)
		goto noframe; /* no need to create a stack frame */

	/* struct return needs special treatment */
	if (ftype == STRTY || ftype == UNIONTY) 
		cerror("eoftn");
	

#if defined(ELFABI)

	sz =  = p2autooff;

	/* calculate the frame space */
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
				addto += SZLONG/SZCHAR;
		}
	}
	
	/* unwind stack frame */	
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
			sz += (SZLONG/SZCHAR);
			if (sz == addto)
				cerror("collision between stack and frame");
			printf("\tlwz %s, -%d(%s)\n", rnames[i], sz, rnames[SPREG]);
		}
	}	
	if (kflag) {
		printf("\tlwz %s,-8(%s)	; restore GOTREG\n", rnames[GOTREG], rnames[FPREG]);
	}
#if defined(FPREG)
	printf("\tlwz %s,-4(%s)	; restore FPREG\n", rnames[FPREG], rnames[FPREG]);
#endif
	
	printf("\tlwz %s,4(%s)	; reload stack pointer\n", rnames[R0], rnames[SPREG]);
	printf("\tmtlr %s		; restore link register\n", rnames[R0]);
	printf("\tlwz %s,0(%s)	; restore stack pointer\n", rnames[SPREG], rnames[SPREG]);
	
	if (p2calls) {
		
		printf("\tlwz %s,4(%s)	; restore condition register\n", rnames[R0], rnames[SPREG]);
		printf("\tmtcr %s\n", rnames[R0]);
		printf("\tlwz %s,8(%s)	; reload link register\n", rnames[R0], rnames[SPREG]);
	
		printf("\tmtlr %s			; restore link register\n", rnames[R0]);
	} else {
		/* restore the R31 we might have used with PIC labels*/
		//printf("\tlwz %s,-4(%s)\n", rnames[R31], rnames[SPREG]);
		printf("\tmr %s, %s\n", rnames[R31], rnames[R0]);
	}
	
	printf("\tlwz %s,4(%s)	; reload stack pointer\n", rnames[R0], rnames[SPREG]);
	printf("\tmtlr %s		; restore link register\n", rnames[R0]);
	printf("\tlwz %s,0(%s)	; restore stack pointer\n", rnames[SPREG], rnames[SPREG]);
	
	
	
	
#elif defined(MACHOABI)

	sz = 4;

	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i))
			idx++; 
	}
	
	if (p2calls > 0)
		idx++;


	/* calculate the frame space */
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
				addto += SZLONG/SZCHAR;
		}
	}
	
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i))
			idx++; 
	}
	
	if (p2calls > 0)
		idx++;



	if (p2calls) {
		/* unwind stack frame */
		printf("\tlwz %s,0(%s)	; restore stack pointer\n", rnames[SPREG], rnames[SPREG]);
		printf("\tlwz %s,-4(%s)	; restore PICREG\n", rnames[PICREG], rnames[SPREG]);
				
		/* restore other registers */

#if 1
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
			sz += (SZLONG/SZCHAR);
			if (sz == addto)
				cerror("collision between stack and frame");
			printf("\tlwz %s, -%d(%s)\n", rnames[i], sz, rnames[SPREG]);
		}
	}

#else
	for (i = (MAXREGS-1); i >= 0; --i) {
		if (TESTBIT(p2env.p_regs, i)) {
			sz = idx*(SZLONG/SZCHAR);
			if (sz == 0)
				cerror("collision unwinding stack");
			printf("\tlwz %s, -%d(%s)\n", rnames[i], sz, rnames[SPREG]);
			--idx; 
		}
	}
#endif

		printf("\tlwz %s,4(%s)	; restore condition register\n", rnames[R0], rnames[SPREG]);
		printf("\tmtcr %s\n", rnames[R0]);
		printf("\tlwz %s,8(%s)	; reload link register\n", rnames[R0], rnames[SPREG]);
	
		printf("\tmtlr %s			; restore link register\n", rnames[R0]);
	} else {
		/* restore the R31 we might have used with PIC labels*/
		//printf("\tlwz %s,-4(%s)\n", rnames[R31], rnames[SPREG]);
		printf("\tmr %s, %s\n", rnames[R31], rnames[R0]);
	}



#elif defined(AOUTABI)

#else
#error unsupported ABI in local2.c:eoftn()

#endif



noframe:	
	printf("\tblr\n");
}

/*
 * add/sub/...
 *
 * Param given:
 */
void
hopcode(int f, int o)
{
	char *str;

	switch (o) {
	case PLUS:
		str = "addw";
		break;
	case MINUS:
		str = "subw";
		break;
	case AND:
		str = "and";
		break;
	case OR:
		str = "or";
		break;
	case ER:
		str = "xor";
		break;
	default:
		comperr("hopcode2: %d", o);
		str = 0; /* XXX gcc */
	}
	printf("%s%c", str, f);
}

/*
 * Return type size in bytes.  Used by R2REGS, arg 2 to offset().
 */
int
tlen(NODE *p)
{
	switch(p->n_type) {
		case CHAR:
		case UCHAR:
			return(1);

		case SHORT:
		case USHORT:
			return(SZSHORT/SZCHAR);

		case DOUBLE:
			return(SZDOUBLE/SZCHAR);

		case INT:
		case UNSIGNED:
		case LONG:
		case ULONG:
			return(SZINT/SZCHAR);

		case LONGLONG:
		case ULONGLONG:
			return SZLONGLONG/SZCHAR;

		default:
			if (!ISPTR(p->n_type))
				comperr("tlen type %d not pointer");
			return SZPOINT(p->n_type)/SZCHAR;
		}
}

/*
 * Emit code to compare two longlong numbers.
 */
static void
twollcomp(NODE *p)
{
	int o = p->n_op;
	int s = getlab2();
	int e = p->n_label;
	int cb1, cb2;

	if (o >= ULE)
		o -= (ULE-LE);
	switch (o) {
	case NE:
		cb1 = 0;
		cb2 = NE;
		break;
	case EQ:
		cb1 = NE;
		cb2 = 0;
		break;
	case LE:
	case LT:
		cb1 = GT;
		cb2 = LT;
		break;
	case GE:
	case GT:
		cb1 = LT;
		cb2 = GT;
		break;
	
	default:
		cb1 = cb2 = 0; /* XXX gcc */
	}
	if (p->n_op >= ULE)
		cb1 += 4, cb2 += 4;
	if (p->n_op >= ULE)
		expand(p, 0, "\tcmplw UL,UR" COM "compare 64-bit values (upper)\n");
	else
		expand(p, 0, "\tcmpw UL,UR" COM "compare 64-bit values (upper)\n");
	if (cb1) cbgen(cb1, s);
	if (cb2) cbgen(cb2, e);
	if (p->n_op >= ULE)
		expand(p, 0, "\tcmplw AL,AR" COM "(and lower)\n");
	else
		expand(p, 0, "\tcmpw AL,AR" COM "(and lower)\n");
	cbgen(p->n_op, e);
	deflab(s);
}

static void
shiftop(NODE *p)
{
	NODE *r = p->n_right;
	TWORD ty = p->n_type;

	if (p->n_op == LS && r->n_op == ICON && getlval(r) < 32) {
		expand(p, INBREG, "\tsrwi A1,AL,32-AR" COM "64-bit left-shift\n");
		expand(p, INBREG, "\tslwi U1,UL,AR\n");
		expand(p, INBREG, "\tor U1,U1,A1\n");
		expand(p, INBREG, "\tslwi A1,AL,AR\n");
	} else if (p->n_op == LS && r->n_op == ICON && getlval(r) < 64) {
		expand(p, INBREG, "\tli A1,0" COM "64-bit left-shift\n");
		if (getlval(r) == 32)
			expand(p, INBREG, "\tmr U1,AL\n");
		else
			expand(p, INBREG, "\tslwi U1,AL,AR-32\n");
	} else if (p->n_op == LS && r->n_op == ICON) {
		expand(p, INBREG, "\tli A1,0" COM "64-bit left-shift\n");
		expand(p, INBREG, "\tli U1,0\n");
	} else if (p->n_op == RS && r->n_op == ICON && getlval(r) < 32) {
		expand(p, INBREG, "\tslwi U1,UL,32-AR" COM "64-bit right-shift\n");
		expand(p, INBREG, "\tsrwi A1,AL,AR\n");
		expand(p, INBREG, "\tor A1,A1,U1\n");
		if (ty == LONGLONG)
			expand(p, INBREG, "\tsrawi U1,UL,AR\n");
		else
			expand(p, INBREG, "\tsrwi U1,UL,AR\n");
	} else if (p->n_op == RS && r->n_op == ICON && getlval(r) < 64) {
		if (ty == LONGLONG)
			expand(p, INBREG, "\tli U1,-1" COM "64-bit right-shift\n");
		else
			expand(p, INBREG, "\tli U1,0" COM "64-bit right-shift\n");
		if (getlval(r) == 32)
			expand(p, INBREG, "\tmr A1,UL\n");
		else if (ty == LONGLONG)
			expand(p, INBREG, "\tsrawi A1,UL,AR-32\n");
		else
			expand(p, INBREG, "\tsrwi A1,UL,AR-32\n");
	} else if (p->n_op == RS && r->n_op == ICON) {
		expand(p, INBREG, "\tli A1,0" COM "64-bit right-shift\n");
		expand(p, INBREG, "\tli U1,0\n");
	}
}

/*
 * Structure assignment.
 */
static void
stasg(NODE *p)
{
	NODE *l = p->n_left;
	int val = getlval(l);

        /* R3 = dest, R4 = src, R5 = len */
        printf("\tli %s,%d\n", rnames[R5], attr_find(p->n_ap, ATTR_P2STRUCT)->iarg(0));
        if (l->n_op == OREG) {
                printf("\taddi %s,%s,%d\n", rnames[R3], rnames[regno(l)], val);
        } else if (l->n_op == NAME) {
#if defined(ELFABI)
                printf("\tli %s,", rnames[R3]);
                adrput(stdout, l);
		printf("@ha\n");
                printf("\taddi %s,%s,", rnames[R3], rnames[R3]);
                adrput(stdout, l);
		printf("@l\n");
#elif defined(MACHOABI)
                printf("\tli %s,ha16(", rnames[R3]);
                adrput(stdout, l);
		printf(")\n");
                printf("\taddi %s,%s,lo16(", rnames[R3], rnames[R3]);
                adrput(stdout, l);
		printf(")\n");
#endif
        }
#if defined(ELFABI)
	if (kflag) {
	        printf("\tbl %s@got(30)\n", EXPREFIX "memcpy");
	} else {
	        printf("\tbl %s\n", EXPREFIX "memcpy");
	}
#elif defined(MACHOABI)
	    printf("\tbl L%s$stub\n", EXPREFIX "memcpy");
		addstub(&stublist, EXPREFIX "memcpy");
#endif
	
}

static void
fpemul(NODE *p)
{
	NODE *l = p->n_left;
	char *ch = NULL;

	if (p->n_op == PLUS && p->n_type == FLOAT) ch = "addsf3";
	else if (p->n_op == PLUS && p->n_type == DOUBLE) ch = "adddf3";
	else if (p->n_op == PLUS && p->n_type == LDOUBLE) ch = "addtf3";

	else if (p->n_op == MINUS && p->n_type == FLOAT) ch = "subsf3";
	else if (p->n_op == MINUS && p->n_type == DOUBLE) ch = "subdf3";
	else if (p->n_op == MINUS && p->n_type == LDOUBLE) ch = "subtf3";

	else if (p->n_op == MUL && p->n_type == FLOAT) ch = "mulsf3";
	else if (p->n_op == MUL && p->n_type == DOUBLE) ch = "muldf3";
	else if (p->n_op == MUL && p->n_type == LDOUBLE) ch = "multf3";

	else if (p->n_op == DIV && p->n_type == FLOAT) ch = "divsf3";
	else if (p->n_op == DIV && p->n_type == DOUBLE) ch = "divdf3";
	else if (p->n_op == DIV && p->n_type == LDOUBLE) ch = "divtf3";

	else if (p->n_op == UMINUS && p->n_type == FLOAT) ch = "negsf2";
	else if (p->n_op == UMINUS && p->n_type == DOUBLE) ch = "negdf2";
	else if (p->n_op == UMINUS && p->n_type == LDOUBLE) ch = "negtf2";

	else if (p->n_op == EQ && l->n_type == FLOAT) ch = "eqsf2";
	else if (p->n_op == EQ && l->n_type == DOUBLE) ch = "eqdf2";
	else if (p->n_op == EQ && l->n_type == LDOUBLE) ch = "eqtf2";

	else if (p->n_op == NE && l->n_type == FLOAT) ch = "nesf2";
	else if (p->n_op == NE && l->n_type == DOUBLE) ch = "nedf2";
	else if (p->n_op == NE && l->n_type == LDOUBLE) ch = "netf2";

	else if (p->n_op == GE && l->n_type == FLOAT) ch = "gesf2";
	else if (p->n_op == GE && l->n_type == DOUBLE) ch = "gedf2";
	else if (p->n_op == GE && l->n_type == LDOUBLE) ch = "getf2";

	else if (p->n_op == LE && l->n_type == FLOAT) ch = "lesf2";
	else if (p->n_op == LE && l->n_type == DOUBLE) ch = "ledf2";
	else if (p->n_op == LE && l->n_type == LDOUBLE) ch = "letf2";

	else if (p->n_op == GT && l->n_type == FLOAT) ch = "gtsf2";
	else if (p->n_op == GT && l->n_type == DOUBLE) ch = "gtdf2";
	else if (p->n_op == GT && l->n_type == LDOUBLE) ch = "gttf2";

	else if (p->n_op == LT && l->n_type == FLOAT) ch = "ltsf2";
	else if (p->n_op == LT && l->n_type == DOUBLE) ch = "ltdf2";
	else if (p->n_op == LT && l->n_type == LDOUBLE) ch = "lttf2";

	else if (p->n_op == SCONV && p->n_type == FLOAT) {
		if (l->n_type == DOUBLE) ch = "truncdfsf2";
		else if (l->n_type == LDOUBLE) ch = "truncdfsf2";
		else if (l->n_type == ULONGLONG) ch = "floatunsdisf";
		else if (l->n_type == LONGLONG) ch = "floatdisf";
		else if (l->n_type == LONG) ch = "floatsisf";
		else if (l->n_type == ULONG) ch = "floatunsisf";
		else if (l->n_type == INT) ch = "floatsisf";
		else if (l->n_type == UNSIGNED) ch = "floatunsisf";
	} else if (p->n_op == SCONV && p->n_type == DOUBLE) {
		if (l->n_type == FLOAT) ch = "extendsfdf2";
		else if (l->n_type == LDOUBLE) ch = "truncdfdf2";
		else if (l->n_type == ULONGLONG) ch = "floatunsdidf";
		else if (l->n_type == LONGLONG) ch = "floatdidf";
		else if (l->n_type == LONG) ch = "floatsidf";
		else if (l->n_type == ULONG) ch = "floatunssidf";
		else if (l->n_type == INT) ch = "floatsidf";
		else if (l->n_type == UNSIGNED) ch = "floatunssidf";
	} else if (p->n_op == SCONV && p->n_type == LDOUBLE) {
		if (l->n_type == FLOAT) ch = "extendsfdf2";
		else if (l->n_type == DOUBLE) ch = "extenddfdf2";
		else if (l->n_type == ULONGLONG) ch = "floatunsdidf";
		else if (l->n_type == LONGLONG) ch = "floatdidf";
		else if (l->n_type == LONG) ch = "floatsidf";
		else if (l->n_type == ULONG) ch = "floatunssidf";
		else if (l->n_type == INT) ch = "floatsidf";
		else if (l->n_type == UNSIGNED) ch = "floatunsidf";
	} else if (p->n_op == SCONV && p->n_type == ULONGLONG) {
		if (l->n_type == FLOAT) ch = "fixunssfdi";
		else if (l->n_type == DOUBLE) ch = "fixunsdfdi";
		else if (l->n_type == LDOUBLE) ch = "fixunsdfdi";
	} else if (p->n_op == SCONV && p->n_type == LONGLONG) {
		if (l->n_type == FLOAT) ch = "fixsfdi";
		else if (l->n_type == DOUBLE) ch = "fixdfdi";
		else if (l->n_type == LDOUBLE) ch = "fixdfdi";
	} else if (p->n_op == SCONV && p->n_type == LONG) {
		if (l->n_type == FLOAT) ch = "fixsfdi";
		else if (l->n_type == DOUBLE) ch = "fixdfdi";
		else if (l->n_type == LDOUBLE) ch = "fixdfdi";
	} else if (p->n_op == SCONV && p->n_type == ULONG) {
		if (l->n_type == FLOAT) ch = "fixunssfdi";
		else if (l->n_type == DOUBLE) ch = "fixunsdfdi";
		else if (l->n_type == LDOUBLE) ch = "fixunsdfdi";
	} else if (p->n_op == SCONV && p->n_type == INT) {
		if (l->n_type == FLOAT) ch = "fixsfsi";
		else if (l->n_type == DOUBLE) ch = "fixdfsi";
		else if (l->n_type == LDOUBLE) ch = "fixdfsi";
	} else if (p->n_op == SCONV && p->n_type == UNSIGNED) {
		if (l->n_type == FLOAT) ch = "fixunssfsi";
		else if (l->n_type == DOUBLE) ch = "fixunsdfsi";
		else if (l->n_type == LDOUBLE) ch = "fixunsdfsi";
	}

	if (ch == NULL) comperr("ZF: op=0x%x (%d)\n", p->n_op, p->n_op);

	if (kflag) {
#if defined(ELFABI)
		printf("\tbl __%s%s@got(30)" COM "soft-float\n", EXPREFIX, ch);
#elif defined(MACHOABI)
		char buf[32];
		printf("\tbl L__%s%s$stub" COM "soft-float\n", EXPREFIX, ch);
		snprintf(buf, 32, "__%s%s", EXPREFIX, ch);
		addstub(&stublist, buf);
#endif
	} else {
		printf("\tbl __%s%s" COM "soft-float\n", EXPREFIX, ch);
	}

	if (p->n_op >= EQ && p->n_op <= GT)
		printf("\tcmpwi %s,0\n", rnames[R3]);
}

/*
 * http://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html#Integer-library-routines
 */

static void
emul(NODE *p)
{
	char *ch = NULL;

        if (p->n_op == LS && DEUNSIGN(p->n_type) == LONGLONG) ch = "ashldi3";
        else if (p->n_op == LS && (DEUNSIGN(p->n_type) == LONG ||
            DEUNSIGN(p->n_type) == INT))
                ch = "ashlsi3";

        else if (p->n_op == RS && p->n_type == ULONGLONG) ch = "lshrdi3";
        else if (p->n_op == RS && (p->n_type == ULONG || p->n_type == INT))
                ch = "lshrsi3";

        else if (p->n_op == RS && p->n_type == LONGLONG) ch = "ashrdi3";
        else if (p->n_op == RS && (p->n_type == LONG || p->n_type == INT))
                ch = "ashrsi3";
        
        else if (p->n_op == DIV && p->n_type == LONGLONG) ch = "divdi3";
        else if (p->n_op == DIV && (p->n_type == LONG || p->n_type == INT))
                ch = "divsi3";

        else if (p->n_op == DIV && p->n_type == ULONGLONG) ch = "udivdi3";
        else if (p->n_op == DIV && (p->n_type == ULONG ||
            p->n_type == UNSIGNED))
                ch = "udivsi3";

        else if (p->n_op == MOD && p->n_type == LONGLONG) ch = "moddi3";
        else if (p->n_op == MOD && (p->n_type == LONG || p->n_type == INT))
                ch = "modsi3";

        else if (p->n_op == MOD && p->n_type == ULONGLONG) ch = "umoddi3";
        else if (p->n_op == MOD && (p->n_type == ULONG ||
            p->n_type == UNSIGNED))
                ch = "umodsi3";

        else if (p->n_op == MUL && p->n_type == LONGLONG) ch = "muldi3";
        else if (p->n_op == MUL && (p->n_type == LONG || p->n_type == INT))
                ch = "mulsi3";

        else if (p->n_op == UMINUS && p->n_type == LONGLONG) ch = "negdi2";
        else if (p->n_op == UMINUS && p->n_type == LONG) ch = "negsi2";

	else ch = 0, comperr("ZE");
	if (kflag) {
#if defined(ELFABI)
		printf("\tbl __%s%s@got(30)" COM "emulated op\n", EXPREFIX, ch);
#elif defined(MACHOABI)
		char buf[32];
		printf("\tbl L__%s%s$stub" COM "emulated op\n", EXPREFIX, ch);
		snprintf(buf, 32, "__%s%s", EXPREFIX, ch);
		addstub(&stublist, buf);
#endif
	} else {
		printf("\tbl __%s%s" COM "emulated operation\n", EXPREFIX, ch);
	}
}

/*
 *  Floating-point conversions (int -> float/double & float/double -> int)
 */

static void
ftoi(NODE *p)
{
	NODE *l = p->n_left;

	printf(COM "start conversion float/(l)double to int\n");

	if (l->n_op != OREG) {
		expand(p, 0, "\tstw AL,-4");
		printf("(%s)\n", rnames[SPREG]);
		if (l->n_type == FLOAT)
			expand(p, 0, "\tlfs A2,");
		else
			expand(p, 0, "\tlfd A2,\n");
		printf("-4(%s)\n", rnames[SPREG]);
	} else {
		if (l->n_type == FLOAT)
			expand(p, 0, "\tlfs A2,AL\n");
		else
			expand(p, 0, "\tlfd A2,AL\n");
	}

	expand(p, 0, "\tfctiwz A2,A2\n");
	expand(p, 0, "\tstfd A2,");
	printf("-8(%s)\n", rnames[SPREG]);
	expand(p, 0, "\tlwz A1,");
	printf("-4(%s)\n", rnames[SPREG]);

	printf(COM "end conversion\n");
}

static void
ftou(NODE *p)
{
	static int lab = 0;
	NODE *l = p->n_left;
	int lab1 = getlab2();
	int lab2 = getlab2();

	printf(COM "start conversion of float/(l)double to unsigned\n");

	if (lab == 0) {
		lab = getlab2();
		expand(p, 0, "\t.data\n");
		printf(LABFMT ":\t.long 0x41e00000\n\t.long 0\n", lab);
		expand(p, 0, "\t.text\n");
	}

	if (l->n_op != OREG) {
		expand(p, 0, "\tstw AL,");
		printf("-4(%s)\n", rnames[SPREG]);
		if (l->n_type == FLOAT)
			expand(p, 0, "\tlfs A3,");
		else
			expand(p, 0, "\tlfd A3,");
		printf("-4(%s)\n", rnames[SPREG]);
		
	} else {
		if (l->n_type == FLOAT)
			expand(p, 0, "\tlfs A3,AL\n");
		else
			expand(p, 0, "\tlfd A3,AL\n");
	}

#if 0
	if (kflag) {
		expand(p, 0, "\taddis A1,");
		printf("%s,ha16(", rnames[R31]);
		printf(LABFMT, lab);
		printf("-L%s$pb)\n", cftnsp->soname ? cftnsp->soname : exname(cftnsp->sname));
       		expand(p, 0, "\tlfd A2,lo16(");
		printf(LABFMT, lab);
		printf("-L%s$pb)\n", cftnsp->soname ? cftnsp->soname : exname(cftnsp->sname));
		expand(p, 0, "(A1)\n");
	} else {
               	expand(p, 0, "\tlfd A2,");
		printf(LABFMT "\n", lab);
	}
#endif

#if defined(ELFABI)

	expand(p, 0, "\taddis A1,");
	printf("%s," LABFMT "@ha\n", rnames[R31], lab);
	expand(p, 0, "\tlfd A2,");
	printf(LABFMT "@l", lab);
	expand(p, 0, "(A1)\n");

#elif defined(MACHOABI)

	expand(p, 0, "\taddis A1,");
	printf("%s,ha16(", rnames[R31]);
	printf(LABFMT, lab);
	printf(")\n");

	expand(p, 0, "\tlfd A2,lo16(");
	printf(LABFMT, lab);
	expand(p, 0, ")(A1)\n");

#endif

	expand(p, 0, "\tfcmpu cr7,A3,A2\n");
	printf("\tcror 30,29,30\n");
	printf("\tbeq cr7,"LABFMT "\n", lab1);

	expand(p, 0, "\tfctiwz A2,A3\n");
	expand(p, 0, "\tstfd A2,");
	printf("-8(%s)\n", rnames[SPREG]);
	expand(p, 0, "\tlwz A1,");
	printf("-4(%s)\n", rnames[SPREG]);
	printf("\tba " LABFMT "\n", lab2);

	deflab(lab1);

        expand(p, 0, "\tfsub A2,A3,A2\n");
        expand(p, 0, "\tfctiwz A2,A2\n");
	expand(p, 0, "\tstfd A2,");
	printf("-8(%s)\n", rnames[SPREG]);
	expand(p, 0, "\tlwz A1,");
	printf("-4(%s)\n", rnames[SPREG]);
        expand(p, 0, "\txoris A1,A1,0x8000\n");

	deflab(lab2);

	printf(COM "end conversion\n");
}

static void
itof(NODE *p)
{
	static int labu = 0;
	static int labi = 0;
	int lab;
	NODE *l = p->n_left;

	printf(COM "start conversion (u)int to float/(l)double\n");

	if (labi == 0 && l->n_type == INT) {
		labi = getlab2();
		expand(p, 0, "\t.data\n");
		printf(LABFMT ":\t.long 0x43300000\n\t.long 0x80000000\n", labi);
		expand(p, 0, "\t.text\n");
	} else if (labu == 0 && l->n_type == UNSIGNED) {
		labu = getlab2();
		expand(p, 0, "\t.data\n");
		printf(LABFMT ":\t.long 0x43300000\n\t.long 0x00000000\n", labu);
		expand(p, 0, "\t.text\n");
	}

	if (l->n_type == INT) {
		expand(p, 0, "\txoris A1,AL,0x8000\n");
		lab = labi;
	} else {
		lab = labu;
	}
	expand(p, 0, "\tstw A1,");
	printf("-4(%s)\n", rnames[SPREG]);
        expand(p, 0, "\tlis A1,0x4330\n");
        expand(p, 0, "\tstw A1,");
	printf("-8(%s)\n", rnames[SPREG]);
        expand(p, 0, "\tlfd A3,");
	printf("-8(%s)\n", rnames[SPREG]);

#if 0
	if (kflag) {
		expand(p, 0, "\taddis A1,");
		printf("%s,ha16(", rnames[R31]);
		printf(LABFMT, lab);
		printf("-L%s$pb)\n", cftnsp->soname ? cftnsp->soname : exname(cftnsp->sname));
       		expand(p, 0, "\tlfd A2,lo16(");
		printf(LABFMT, lab);
		printf("-L%s$pb)\n", cftnsp->soname ? cftnsp->soname : exname(cftnsp->sname));
		expand(p, 0, "(A1)\n");
	} else {
               	expand(p, 0, "\tlfd A2,");
		printf(LABFMT "\n", lab);
	}
#endif

#if defined(ELFABI)

	expand(p, 0, "\taddis A1,");
	printf("%s," LABFMT "@ha\n", rnames[R31], lab);
       	expand(p, 0, "\tlfd A2,");
	printf(LABFMT "@l", lab);
	expand(p, 0, "(A1)\n");

#elif defined(MACHOABI)

	expand(p, 0, "\taddis A1,");
	printf("%s,ha16(", rnames[R31]);
	printf(LABFMT, lab);

	printf(")\n");
	expand(p, 0, "\tlfd A2,lo16(");
	printf(LABFMT, lab);

	expand(p, 0, ")(A1)\n");

#endif

	expand(p, 0, "\tfsub A3,A3,A2\n");
	if (p->n_type == FLOAT)
		expand(p, 0, "\tfrsp A3,A3\n");

	printf(COM "end conversion\n");
}


static void
fpconv(NODE *p)
{
	NODE *l = p->n_left;

#ifdef PCC_DEBUG
	if (p->n_op != SCONV)
		cerror("fpconv 1");
#endif

	if (DEUNSIGN(l->n_type) == INT)
		itof(p);
	else if (p->n_type == INT)
		ftoi(p);
	else if (p->n_type == UNSIGNED)
		ftou(p);
	else
		cerror("unhandled floating-point conversion");

}

void
zzzcode(NODE *p, int c)
{
	switch (c) {

	case 'C': /* floating-point conversions */
		fpconv(p);
		break;

	case 'D': /* long long comparision */
		twollcomp(p);
		break;

	case 'E': /* print out emulated ops */
		emul(p);
		break;

	case 'F': /* print out emulate floating-point ops */
		fpemul(p);
		break;

	case 'O': /* 64-bit left and right shift operators */
		shiftop(p);
		break;

	case 'Q': /* emit struct assign */
		stasg(p);
		break;

	default:
		comperr("zzzcode %c", c);
	}
}

/*ARGSUSED*/
int
rewfld(NODE *p)
{
	return(1);
}

int canaddr(NODE *);
int
canaddr(NODE *p)
{
	int o = p->n_op;

	if (o == NAME || o == REG || o == ICON || o == OREG ||
	    (o == UMUL && shumul(p->n_left, SOREG)))
		return(1);
	return 0;
}

int
fldexpand(NODE *p, int cookie, char **cp)
{
	CONSZ val;
	int shft;

	if (p->n_op == ASSIGN)
		p = p->n_left;

	if (features(FEATURE_BIGENDIAN))
		shft = SZINT - UPKFSZ(p->n_rval) - UPKFOFF(p->n_rval);
	else
		shft = UPKFOFF(p->n_rval);

	switch (**cp) {
	case 'S':
		printf("%d", UPKFSZ(p->n_rval));
		break;
	case 'H':
		printf("%d", shft);
		break;
	case 'M':
	case 'N':
		val = (CONSZ)1 << UPKFSZ(p->n_rval);
		--val;
		val <<= shft;
		printf(CONFMT, (**cp == 'M' ? val : ~val)  & 0xffffffff);
		break;
	default:
		comperr("fldexpand");
	}
	return 1;
}

/*
 * Does the bitfield shape match?
 */
int
flshape(NODE *p)
{
	int o = p->n_op;

	if (o == OREG || o == REG || o == NAME)
		return SRDIR; /* Direct match */
	if (o == UMUL && shumul(p->n_left, SOREG))
		return SROREG; /* Convert into oreg */
	return SRREG; /* put it into a register */
}

/* INTEMP shapes must not contain any temporary registers */
/* XXX should this go away now? */
int
shtemp(NODE *p)
{
	printf("; shtemp\n");
	return 0;
#if 0
	int r;

	if (p->n_op == STARG )
		p = p->n_left;

	switch (p->n_op) {
	case REG:
		return (!istreg(regno(p)));

	case OREG:
		r = regno(p);
		if (R2TEST(r)) {
			if (istreg(R2UPK1(r)))
				return(0);
			r = R2UPK2(r);
		}
		return (!istreg(r));

	case UMUL:
		p = p->n_left;
		return (p->n_op != UMUL && shtemp(p));
	}

	if (optype(p->n_op) != LTYPE)
		return(0);
	return(1);
#endif
}

void
adrcon(CONSZ val)
{
	printf( CONFMT, val);
}

void
conput(FILE *fp, NODE *p)
{
	int val = getlval(p);

	switch (p->n_op) {
	case ICON:
		if (p->n_name[0] != '\0') {
			fprintf(fp, "%s", p->n_name);
			if (val)
				fprintf(fp, "+%d", val);
		} else {
			if (GCLASS(p->n_type) == CLASSB)
				fprintf(fp, CONFMT, getlval(p) >> 32);
			else
				fprintf(fp, "%d", val);
		}
		return;

	default:
		comperr("illegal conput, p %p", p);
	}
}

/*ARGSUSED*/
void
insput(NODE *p)
{
	comperr("insput");
}

/*
 * Print lower or upper name of 64-bit register.
 */
static void
reg64name(int reg, int hi)
{
	int idx;
	int off = 0;

	idx = (reg > R14R15 ? (2*(reg - R14R15) + R14) : (reg - R3R4 + R3));

	if ((hi == HIREG && !features(FEATURE_BIGENDIAN)) ||
	    (hi == LOWREG && features(FEATURE_BIGENDIAN)))
		off = 1;
		
	printf("%s" , rnames[idx + off]);
}

/*
 * Write out the upper address, like the upper register of a 2-register
 * reference, or the next memory location.
 */
void
upput(NODE *p, int size)
{
//	size /= SZCHAR;
	switch (p->n_op) {
	case REG:
		reg64name(regno(p), HIREG);
		break;

	case NAME:
	case OREG:
		if (features(FEATURE_BIGENDIAN))
			printf("%d", (int)getlval(p));
		else
			printf("%d", (int)(getlval(p) + 4));
		printf("(%s)", rnames[regno(p)]);
		break;

	case ICON:
		printf(CONFMT, getlval(p) >> size);
		break;

	default:
		comperr("upput bad op %d size %d", p->n_op, size);
	}
}

void
adrput(FILE *io, NODE *p)
{
	/* output an address, with offsets, from p */

	if (p->n_op == FLD)
		p = p->n_left;

	switch (p->n_op) {

	case NAME:
		if (p->n_name[0] != '\0') {
#if !(defined(MACHOABI))
			if (kflag)
				fprintf(io, "L%s$pb-", &p2env.ipp->ipp_name[1]);
			fputs(p->n_name, io);
#else
			/* MACHOABI requires kflag */
			fputs(p->n_name, io);
			fprintf(io, "-L%s$pb", &p2env.ipp->ipp_name[1]);
#endif
			if (getlval(p) != 0)
				fprintf(io, "+" CONFMT, getlval(p));
		} else
			fprintf(io, CONFMT, getlval(p));
		return;

	case OREG:
		if (DEUNSIGN(p->n_type) == LONGLONG &&
		    features(FEATURE_BIGENDIAN))
			fprintf(io, "%d", (int)getlval(p) + 4);
		else
			fprintf(io, "%d", (int)getlval(p));
		fprintf(io, "(%s)", rnames[regno(p)]);
		return;

	case ICON:
		/* addressable value of the constant */
		conput(io, p);
		return;

	case REG:
	//printf("xxxxx reg %d\n", regno(p));
		if (GCLASS(regno(p)) == CLASSB)
			reg64name(regno(p), LOWREG);
		else
			fprintf(io, "%s", rnames[regno(p)]);
#if 0
		switch (p->n_type) {
		case DOUBLE:
		case LDOUBLE:
			if (features(FEATURE_HARDFLOAT)) {
				fprintf(io, "%s", rnames[regno(p)]);
				break;
			}
			/* FALL-THROUGH */
		case LONGLONG:
		case ULONGLONG:
			reg64name(regno(p), LOWREG);
			break;
		default:
			fprintf(io, "%s", rnames[regno(p)]);
		}
#endif
		return;

	default:
		comperr("illegal address, op %d, node %p", p->n_op, p);
		return;

	}
}

/*
 * these mnemonics match the order of the preprocessor decls
 * EQ, NE, LE, LT, GE, GT, ULE, ULT, UGE, UGT
 */

static char *
ccbranches[] = {
	"beq",		/* branch if equal */
	"bne",		/* branch if not-equal */
	"ble",		/* branch if less-than-or-equal */
	"blt",		/* branch if less-than */
	"bge",		/* branch if greater-than-or-equal */
	"bgt",		/* branch if greater-than */
	/* what should these be ? */
	"ble",		/* branch if less-than-or-equal */
	"blt",		/* branch if less-than */
	"bge",		/* branch if greater-than-or-equal */
	"bgt",		/* branch if greater-than */

};


/*   printf conditional and unconditional branches */
void
cbgen(int o, int lab)
{
	if (o < EQ || o > UGT)
		comperr("bad conditional branch: %s", opst[o]);
	printf("\t%s " LABFMT "\n", ccbranches[o-EQ], lab);
}


static int
argsize(NODE *p, n_regs *nr)
{
	TWORD t = p->n_type;

	if (t == FLOAT) {
		nr->fpr += 1;
		return 4;
	}		
	if (t < LONGLONG ||  t > BTMASK){
		nr->gpr += 1;
		return 4;
	}
	if (t == LONGLONG || t == ULONGLONG) {
		nr->gpr += 2;
		return 8;
	}
	if (t == DOUBLE || t == LDOUBLE) {
		nr->fpr += (t == DOUBLE ? 1 : 2);
		return (t == DOUBLE ? 4 : 8);
	}
	if (t == STRTY || t == UNIONTY) {
		nr->gpr += 1;
		return attr_find(p->n_ap, ATTR_P2STRUCT)->iarg(0);
	}
	comperr("argsize");
	return 0;
}

static int
calc_args_size(NODE *p, n_regs *nr)
{
	int n = 0;
        
        if (p->n_op == CM) {
                //n += calc_args_size(p->n_left, nr);
                n += calc_args_size(p->n_right, nr);
                /* uncount the function call */
                nr->gpr -= 1;
                return n;
        }

        n += argsize(p, nr);

        return n;
}


static void
fixcalls(NODE *p, void *arg)
{
	int n = 0;

	switch (p->n_op) {
	case STCALL:
	case CALL:
		n = calc_args_size(p->n_right, arg);
		if (n > p2maxstacksize)
			p2maxstacksize = n;
		/* FALLTHROUGH */
	case USTCALL:
	case UCALL:
		++p2calls;
		break;
	case TEMP:
		p2temps += argsize(p, arg);
		break;
	}
}

/*
 * Must store floats in memory if there are two function calls involved.
 */
static int
storefloat(struct interpass *ip, NODE *p)
{
	int l, r;

  switch (optype(p->n_op)) {
	case BITYPE:
		l = storefloat(ip, p->n_left);
		r = storefloat(ip, p->n_right);
		if (p->n_op == CM)
			return 0; /* arguments, don't care */
		if (callop(p->n_op))
			return 1; /* found one */

#define ISF(p) ((p)->n_type == FLOAT || (p)->n_type == DOUBLE || \
(p)->n_type == LDOUBLE)
	
		if (ISF(p->n_left) && ISF(p->n_right) && l && r) {
			/* must store one. store left */
			struct interpass *nip;
			TWORD t = p->n_left->n_type;
			NODE *ll;
			int off;

			off = (freetemp(szty(t)));
			ll = mklnode(OREG, off, SPREG, t);
			nip = ipnode(mkbinode(ASSIGN, ll, p->n_left, t));
			p->n_left = mklnode(OREG, off, SPREG, t);
			DLIST_INSERT_BEFORE(ip, nip, qelem);
		}
		return l|r;

	case UTYPE:
		l = storefloat(ip, p->n_left);
		if (callop(p->n_op))
			l = 1;
		return l;

	default:
		return 0;
  }
}

void
myreader(struct interpass *ipole)
{
	struct interpass *ip;
	int i, max, max_gpr = 0, max_fpr = 0;
	struct n_regs nr = {0, 0};

	p2calls = 0;
	p2temps = 0;
	p2maxstacksize = 0;
	p2framesize = 0;


	DLIST_FOREACH(ip, ipole, qelem) {
		if (ip->type != IP_NODE)
			continue;
		walkf(ip->ip_node, fixcalls, &nr);
#ifdef PCC_DEBUG
	if (x2debug)
		printf("nr->gpr = %d, nr->fpr = %d\n", nr.gpr, nr.fpr);
#endif
		if (( nr.gpr > max_gpr) || (nr.fpr > max_fpr )) {
			/* have to sync max_fpr with max _gpr */
			max_gpr = nr.gpr; 
			max_fpr = nr.fpr;
			if (max_fpr > NFPARGREGS)
				comperr("too many floating point args\n");
		}
		nr.gpr = nr.fpr = 0;
		storefloat(ip, ip->ip_node);
	}

//	if (p2maxstacksize < NARGREGS*SZINT/SZCHAR)
	//	p2maxstacksize = NARGREGS*SZINT/SZCHAR;


#ifdef PCC_DEBUG
	if (x2debug)
		printf("max_gpr = %d, max_fpr = %d\n", max_gpr, max_fpr);
#endif
	
#ifndef MACHOABI
	/* calculate the frame space */
	for (i = 0; i < MAXREGS; i++) {
		if (TESTBIT(p2env.p_regs, i)) {
				p2framesize += SZLONG/SZCHAR;
		}
	}
#else
	p2framesize = NARGREGS*(SZINT/SZCHAR);
	p2framesize += NFPARGREGS*(SZDOUBLE/SZCHAR);
#endif
	
	if (p2calls != 0) {
#ifndef MACHOABI 
		p2framesize += 8;	/* stack ptr / return addr */
#else
		p2framesize += 12;	/* stack ptr / condition register/ return addr */
#endif
#if defined(FPREG)
		p2framesize += 4;	/* FPREG/R30 */
#endif
#if defined(ELFABI) || defined(MACHOABI)
		p2framesize += 4;	/* GOTREG/R31 */
#endif
	}
	
	if (max_gpr > NARGREGS) {
		p2framesize += (SZINT/SZCHAR)*(max_gpr - NARGREGS);		
	}
	
	p2framesize += p2maxautooff;		/* autos */
	p2framesize += p2temps;			/* TEMPs that aren't autos */
	
	/* create the new stack frame */	
	p2framesize = (p2framesize+15) & ~15; /* 16-byte aligned */
	//p2maxautooff = p2framesize;
	
#if 1
	printf(";;; MYREADER\n");
	printf(";;; p2maxautooff = %d\n", p2maxautooff);
	printf(";;; p2autooff = %d\n", p2autooff);
	printf(";;; p2temps = %d\n", p2temps);
	printf(";;; p2calls = %d\n", p2calls);
	printf(";;; p2maxstacksize = %d\n", p2maxstacksize);
	printf(";;; p2framesize = %d\n", p2framesize);
#endif

	if (x2debug)
		printip(ipole);
}

/*
 * Remove some PCONVs after OREGs are created.
 */
static void
pconv2(NODE *p, void *arg)
{
	NODE *q;

	if (p->n_op == PLUS) {
		if (p->n_type == (PTR|SHORT) || p->n_type == (PTR|USHORT)) {
			if (p->n_right->n_op != ICON)
				return;
			if (p->n_left->n_op != PCONV)
				return;
			if (p->n_left->n_left->n_op != OREG)
				return;
			q = p->n_left->n_left;
			nfree(p->n_left);
			p->n_left = q;
			/*
			 * This will be converted to another OREG later.
			 */
		}
	}
}

void
mycanon(NODE *p)
{
	walkf(p, pconv2, 0);
}

void
myoptim(struct interpass *ip)
{
#ifdef PCC_DEBUG
	if (x2debug) {
		printf("myoptim\n");
	}
#endif
}

/*
 * Move data between registers.  While basic registers aren't a problem,
 * we have to handle the special case of overlapping composite registers.
 * It might just be easier to modify the register allocator so that
 * moves between overlapping registers isn't possible.
 */
void
rmove(int s, int d, TWORD t)
{
	switch (t) {
	case LDOUBLE:
	case DOUBLE:
		if (features(FEATURE_HARDFLOAT)) {
			printf("\tfmr %s,%s" COM "rmove\n",
			    rnames[d], rnames[s]);
			break;
		}
		/* FALL-THROUGH */
	case LONGLONG:
	case ULONGLONG:
		if (s == d+1) {
			/* dh = sl, copy low word first */
			printf("\tmr ");
			reg64name(d, LOWREG);
			printf(",");
			reg64name(s, LOWREG);
			printf("\n");
			printf("\tmr ");
			reg64name(d, HIREG);
			printf(",");
			reg64name(s, HIREG);
			printf("\n");
		} else {
			/* copy high word first */
			printf("\tmr ");
			reg64name(d, HIREG);
			printf(",");
			reg64name(s, HIREG);
			printf("\n");
			printf("\tmr ");
			reg64name(d, LOWREG);
			printf(",");
			reg64name(s, LOWREG);
			printf("\n");
		}
		break;
	case FLOAT:
		if (features(FEATURE_HARDFLOAT)) {
			printf("\tfmr %s,%s" COM "rmove\n",
			    rnames[d], rnames[s]);
			break;
		}
		/* FALL-THROUGH */
	default:
		printf("\tmr %s,%s" COM "rmove\n", rnames[d], rnames[s]);
	}
}

/*
 * For class c, find worst-case displacement of the number of
 * registers in the array r[] indexed by class.
 *
 * On PowerPC, we have:
 *
 * 32 32-bit registers (2 reserved)
 * 16 64-bit pseudo registers
 * 32 floating-point registers
 */
int
COLORMAP(int c, int *r)
{
	int num = 0;

        switch (c) {
        case CLASSA:
		num += r[CLASSA];
		num += 2*r[CLASSB];
#if defined(ELFABI)
		return num < 30;
#elif defined(MACHOABI)
		return num < 28;
#else
#error Need values in CLASSA in COLORMAP for unknown ABI
#endif
        case CLASSB:
                num += 2*r[CLASSB];
                num += r[CLASSA];
                return num < 16;
	case CLASSC:
		return num < 31;
        case CLASSD:
                return r[CLASSD] < DREGCNT;
        }
        return 0; /* XXX gcc */
}

/*
 * Return a class suitable for a specific type.
 */
int
gclass(TWORD t)
{
	if (t == LONGLONG || t == ULONGLONG)
		return CLASSB;
	if (t == FLOAT || t == DOUBLE || t == LDOUBLE) {
		if (features(FEATURE_HARDFLOAT))
			return CLASSC;
		if (t == FLOAT)
			return CLASSA;
		else
			return CLASSB;
	}
	return CLASSA;
}

int
retreg(int t)
{
	int c = gclass(t);
	if (c == CLASSB)
		return R3R4;
	else if (c == CLASSC)
		return F1;
	return R3;
}

/*
 * Calculate argument sizes.
 */
void
lastcall(NODE *p)
{
	NODE *op = p;
	int size = 0;
	struct n_regs nr = {0, 0};

#ifdef PCC_DEBUG
	if (x2debug)
		printf("lastcall:\n");
#endif

	p->n_qual = 0;
	if (p->n_op != CALL && p->n_op != FORTCALL && p->n_op != STCALL)
		return;

	for (p = p->n_right; p->n_op == CM; p = p->n_left)
		size += argsize(p->n_right, &nr);
	size += argsize(p, &nr);
	op->n_qual = size; /* XXX */
}

/*
 * Special shapes.
 */
int
special(NODE *p, int shape)
{
	int o = p->n_op;

	switch (shape) {
	case SFUNCALL:
		if (o == STCALL || o == USTCALL)
			return SRREG;
		break;
	case SPCON:
		if (o == ICON && p->n_name[0] == 0 && (getlval(p) & ~0x7fff) == 0)
			return SRDIR;
		break;
	}
	return SRNOPE;
}

static int fset = FEATURE_BIGENDIAN | FEATURE_HARDFLOAT;

/*
 * Target-dependent command-line options.
 */
void
mflags(char *str)
{
	if (strcasecmp(str, "big-endian") == 0) {
		fset |= FEATURE_BIGENDIAN;
	} else if (strcasecmp(str, "little-endian") == 0) {
		fset &= ~FEATURE_BIGENDIAN;
	} else if (strcasecmp(str, "soft-float") == 0) {
		fset &= ~FEATURE_HARDFLOAT;
	} else if (strcasecmp(str, "hard-float") == 0) {
		fset |= FEATURE_HARDFLOAT;
	} else {
		fprintf(stderr, "unknown m option '%s'\n", str);
		exit(1);
	}
}

int
features(int mask)
{
	return ((fset & mask) == mask);
}
/*
 * Do something target-dependent for xasm arguments.
 * Supposed to find target-specific constraints and rewrite them.
 */
int
myxasm(struct interpass *ip, NODE *p)
{
	return 0;
}
