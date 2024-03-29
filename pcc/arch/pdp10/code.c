/*	$Id: code.c,v 1.45 2023/08/12 09:16:16 ragge Exp $	*/
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


# include "pass1.h"

#ifdef LANG_CXX
#define p1listf listf
#define p1tfree tfree
#define p1nfree nfree
#else
#define NODE P1ND
#define talloc p1alloc
#define	sap sss
#define	n_ap pss
#undef	n_df
#define	n_df pdf
#define	n_type ptype
#endif

/*
 * Print out assembler segment name.
 */
void
setseg(int seg, char *name)
{
	switch (seg) {
	case PROG: name = ".text"; break;
	case DATA:
	case LDATA: name = ".data"; break;
	case UDATA: break;
	case STRNG:
	case RDATA: name = ".section .rodata"; break;
	case NMSEG: 
		printf(PRTPREF "\t.section %s,\"a%c\",@progbits\n", name,
		    cftnsp ? 'x' : 'w');
		return;
	default:
		cerror("bad seg %d", seg);
	}
	printf(PRTPREF "\t%s\n", name);
}

/*
 * Define everything needed to print out some data (or text).
 * This means segment, alignment, visibility, etc.
 */
void
defloc(struct symtab *sp)
{

	if (sp->sclass == EXTDEF)
		printf("	.globl %s\n", getexname(sp));
	if (sp->slevel == 0)
		printf("%s:\n", getexname(sp));
	else
		printf(LABFMT ":\n", sp->soffset);
}

/*
 * code for the end of a function
 */
void
efcode(void)
{
}

/*
 * code for the beginning of a function; a is an array of
 * indices in stab for the arguments; n is the number
 */
void
bfcode(struct symtab **sp, int cnt)
{
	NODE *p, *q;
	int i, n;

	if (cftnsp->stype == STRTY+FTN || cftnsp->stype == UNIONTY+FTN) {
		uerror("no struct return yet");
	}
	/* recalculate the arg offset and create TEMP moves */
	for (n = 1, i = 0; i < cnt; i++) {
		if (n < 8) {
			p = tempnode(0, sp[i]->stype, sp[i]->sdf, sp[i]->sap);
			q = block(REG, NIL, NIL,
			    sp[i]->stype, sp[i]->sdf, sp[i]->sap);
			q->n_rval = n;
			p = buildtree(ASSIGN, p, q);
			sp[i]->soffset = regno(p->n_left);
			sp[i]->sflags |= STNODE;
			ecomp(p);
		} else {
			sp[i]->soffset += SZINT * n;
			if (xtemps) {
				/* put stack args in temps if optimizing */
				p = tempnode(0, sp[i]->stype,
				    sp[i]->sdf, sp[i]->sap);
				p = buildtree(ASSIGN, p, nametree(sp[i]));
				sp[i]->soffset = regno(p->n_left);
				sp[i]->sflags |= STNODE;
				ecomp(p);
			}
		}
		n += szty(sp[i]->stype);
	}
}


void
bjobcode(void)
{
}

/* called just before final exit */
/* flag is 1 if errors, 0 if none */
void
ejobcode(int flag)
{
}

/*
 * Make a register node, helper for funcode.
 */
static NODE *
mkreg(NODE *p, int n)
{
	NODE *r;

	r = block(REG, NIL, NIL, p->n_type, p->n_df, p->n_ap);
	if (szty(p->n_type) == 2)
		n += 16;
	r->n_rval = n;
	return r;
}

static int regnum;
/*
 * Move args to registers and emit expressions bottom-up.
 */
static void
fixargs(NODE *p)
{
	NODE *r;

	if (p->n_op == CM) {
		fixargs(p->n_left);
		r = p->n_right;
		if (r->n_op == STARG)
			regnum = 9; /* end of register list */
		else if (regnum + szty(r->n_type) > 8)
			p->n_right = block(FUNARG, r, NIL, r->n_type,
			    r->n_df, r->n_ap);
		else
			p->n_right = buildtree(ASSIGN, mkreg(r, regnum), r);
	} else {
		if (p->n_op == STARG) {
			regnum = 9; /* end of register list */
		} else {
			r = talloc();
			*r = *p;
			r = buildtree(ASSIGN, mkreg(r, regnum), r);
			*p = *r;
			p1nfree(r);
		}
		r = p;
	}
	regnum += szty(r->n_type);
}


/*
 * Called with a function call with arguments as argument.
 * This is done early in buildtree() and only done once.
 */
NODE *
funcode(NODE *p)
{

	regnum = 1;

	fixargs(p->n_right);
	return p;
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

/*
 * Return "canonical frame address".
 */
NODE *
builtin_cfa(const struct bitable *bt, NODE *a)
{
	uerror(__func__);
	return bcon(0);
}

NODE *
builtin_return_address(const struct bitable *bt, NODE *a)
{
	uerror(__func__);
	return bcon(0);
}

NODE *
builtin_frame_address(const struct bitable *bt, NODE *a)
{
	uerror(__func__);
	return bcon(0);
}

