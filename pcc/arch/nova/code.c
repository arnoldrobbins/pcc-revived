/*	$Id: code.c,v 1.16 2023/08/12 08:54:51 ragge Exp $	*/
/*
 * Copyright (c) 2006 Anders Magnusson (ragge@ludd.luth.se).
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


# include "pass1.h"

#ifdef LANG_CXX
#define P1ND NODE
#define p1alloc talloc
#define p1nfree nfree
#define p1fwalk fwalk
#define p1tcopy ccopy
#define p1listf listf
#else
#define	sap sss
#define	n_type ptype
#undef	n_ap
#define	n_ap pss
#undef	n_df
#define	n_df pdf
#endif

/*
 * cause the alignment to become a multiple of n
 * never called for text segment.
 */
void
defalign(int n)
{
	/* alignment are always correct */
}

/*
 * Print out assembler segment name.
 */
void
setseg(int seg, char *name)
{
#ifndef os_none
	switch (seg) {
	case PROG: name = ".text"; break;
	case DATA:
	case LDATA: name = ".data"; break;
	case STRNG:
	case RDATA: name = ".rodata"; break;
	case UDATA: break;
	default:
		cerror((char *)__func__);
	}
#else
	name = ".NREL";
#endif
	printf("\t%s\n", name);
}

/*
 * Define everything needed to print out some data (or text).
 * This means segment, alignment, visibility, etc.
 */
void
defloc(struct symtab *sp)
{
	char *name;

	name = getexname(sp);

	if (ISFTN(sp->stype))
		return;
	if (sp->sclass == EXTDEF)
		printf("\t.globl %s\n", name);
	if (sp->slevel == 0)
		printf("%s:\n", name);
	else
		printf(LABFMT ":\n", sp->soffset);
}

/* make a common declaration for id, if reasonable */
void
defzero(struct symtab *sp)
{
	int off, al;
	char *name;

	name = getexname(sp);
	off = tsize(sp->stype, sp->sdf, sp->sap);
	SETOFF(off,SZCHAR);
	off /= SZCHAR;
	al = talign(sp->stype, sp->sap)/SZCHAR;

	if (sp->sclass == STATIC) {
		if (sp->slevel == 0) {
			printf("\t.local %s\n", name);
		} else
			printf("\t.local " LABFMT "\n", sp->soffset);
	}
	if (sp->slevel == 0) {
		printf("\t.comm %s,0%o,%d\n", name, off, al);
	} else
		printf("\t.comm " LABFMT ",0%o,%d\n", sp->soffset, off, al);
}


//static int ac3temp;
/*
 * code for the end of a function
 * deals with struct return here
 */
void
efcode(void)
{
	P1ND *p, *q;
//	int sz;

#if 0
	/* restore ac3 */
	p = block(REG, 0, 0, INT, 0, 0);
	regno(p) = 3;
	q = tempnode(ac3temp, INT, 0, 0);
	ecomp(buildtree(ASSIGN, p, q));
#endif


	if (cftnsp->stype != STRTY+FTN && cftnsp->stype != UNIONTY+FTN)
		return;
cerror("efcode");
	/* address of return struct is in eax */
	/* create a call to memcpy() */
	/* will get the result in eax */
	p = block(REG, NULL, NULL, CHAR+PTR, 0, 0);
//	p->n_rval = EAX;
	q = block(OREG, NULL, NULL, CHAR+PTR, 0, 0);
//	q->n_rval = EBP;
	slval(q, 8); /* return buffer offset */
	p = block(CM, q, p, INT, 0, 0);
//	sz = (tsize(STRTY, cftnsp->sdf, cftnsp->ssue)+SZCHAR-1)/SZCHAR;
//	p = block(CM, p, bcon(sz), INT, 0, 0);
	p->n_right->n_name = "";
	p = block(CALL, bcon(0), p, CHAR+PTR, 0, 0);
	p->n_left->n_name = "memcpy";
	p = clocal(p);
	send_passt(IP_NODE, p);
}

/*
 * code for the beginning of a function; a is an array of
 * indices in symtab for the arguments; n is the number
 */
void
bfcode(struct symtab **a, int n)
{
//	P1ND *p, *q;
	int i;

	for (i = 0; i < n; i++) {
		if (a[i]->stype == CHAR)
			a[i]->stype = INT;
		if (a[i]->stype == UCHAR)
			a[i]->stype = UNSIGNED;
	}

	if (cftnsp->stype != STRTY+FTN && cftnsp->stype != UNIONTY+FTN)
		return;
cerror("bfcode");
	/* Function returns struct, adjust arg offset */
	for (i = 0; i < n; i++)
		a[i]->soffset += SZPOINT(INT);
}


/* called just before final exit */
/* flag is 1 if errors, 0 if none */
void
ejobcode(int flag)
{
#ifdef os_none
	printf("\t.END\n");
#endif
}

void
bjobcode(void)
{
#ifdef os_none
	printf("\t.TITL foo\n");
	printf("\t.TXTM 1\n");	/* big endian */
	printf("\t.NREL\n");	/* relocatable */
	printf("\t.EXTU\n");	/* undefined syms are external */
#endif
}

/* fix up type of field p */
void
fldty(struct symtab *p)
{
}

/*
 * XXX - fix genswitch.
 */
int
mygenswitch(int num, TWORD type, struct swents **p, int n)
{
	return 0;
}

static int xoff;

static void
fnummer(P1ND *p)
{
	if (p->n_op != FUNARG)
		return;
	p->n_rval = xoff;
	xoff -= tsize(p->n_type, p->n_df, p->n_ap)/SZSHORT;
}

/*
 * Called with a function call with arguments as argument.
 * This is done early in buildtree() and only done once.
 */
P1ND *
funcode(P1ND *p)
{
	P1ND *r, *l;

	/* Fix function call arguments. On nova, just add funarg */
	for (r = p->n_right; r->n_op == CM; r = r->n_left) {
		if (r->n_right->n_op != STARG) {
			r->n_right = block(FUNARG, r->n_right, NULL,
			    r->n_right->n_type, r->n_right->n_df,
			    r->n_right->n_ap);
		}
	}
	if (r->n_op != STARG) {
		l = p1alloc();
		*l = *r;
		r->n_op = FUNARG;
		r->n_left = l;
		r->n_type = l->n_type;
	}

	xoff = -2;
	p1listf(p->n_right, fnummer);
	return p;
}

/*
 * Return return as given by a.
 */
P1ND *
builtin_return_address(const struct bitable *bt, P1ND *a)
{
	cerror((char *)__func__);
	return 0;
}

/*
 * Return frame as given by a.
 */
P1ND *
builtin_frame_address(const struct bitable *bt, P1ND *a)
{
	cerror((char *)__func__);
	return 0;
}

/*
 * Return "canonical frame address".
 */
P1ND *
builtin_cfa(const struct bitable *bt, P1ND *a)
{
	cerror((char *)__func__);
	return 0;
}

