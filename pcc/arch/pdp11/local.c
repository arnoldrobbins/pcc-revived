/*	$Id: local.c,v 1.26 2023/08/12 09:50:48 ragge Exp $	*/
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

#include "unicode.h"
#include "pass1.h"

#ifndef LANG_CXX
#define	NODE P1ND
#undef NIL
#define NIL NULL
#define fwalk p1fwalk
#define nfree p1nfree
#define	n_type	ptype
#undef n_df
#define	n_df pdf
#undef n_ap
#define	n_ap pss
#define	sap sss
#endif

/*	this file contains code which is dependent on the target machine */

/* clocal() is called to do local transformations on
 * an expression tree preparitory to its being
 * written out in intermediate code.
 *
 * the major essential job is rewriting the
 * automatic variables and arguments in terms of
 * REG and OREG nodes
 * conversion ops which are not necessary are also clobbered here
 * in addition, any special features (such as rewriting
 * exclusive or) are easily handled here as well
 */
NODE *
clocal(NODE *p)
{

	CONSZ c;
	register struct symtab *q;
	register NODE *r, *l;
	register int o;
	TWORD m;

#ifdef PCC_DEBUG
	if (xdebug) {
		printf("clocal: %p\n", p);
		fwalk(p, eprint, 0);
	}
#endif
	switch( o = p->n_op ){

	case NAME:
		if ((q = p->n_sp) == NULL)
			return p; /* Nothing to care about */

		switch (q->sclass) {

		case PARAM:
		case AUTO:
			/* fake up a structure reference */
			r = block(REG, NIL, NIL, PTR+STRTY, 0, 0);
			slval(r, 0);
			r->n_rval = FPREG;
			p = stref(block(STREF, r, p, 0, 0, 0));
			break;

		case STATIC:
			if (q->slevel == 0)
				break;
			slval(p, 0);
			break;

		case REGISTER:
			p->n_op = REG;
			slval(p, 0);
			p->n_rval = q->soffset;
			break;

		case EXTERN:
		case EXTDEF:
			break;
		}
		break;

	case CBRANCH:
		l = p->n_left;
		if (coptype(l->n_op) != BITYPE)
			break;
		if (l->n_left->n_op != SCONV || l->n_right->n_op != ICON)
			break;
		if ((r = l->n_left->n_left)->n_type > INT)
			break;
		c = glval(l->n_right);
		if (c < MIN_INT || c > MAX_UNSIGNED)
			break;
		/* compare with constant without casting */
		nfree(l->n_left);
		l->n_left = r;
		l->n_right->n_type = l->n_left->n_type;
		break;

	case SCONV:
		if (p->n_left->n_op == COMOP)
			break;	/* may propagate wrong type later */
		l = p->n_left;

		if (p->n_type == l->n_type) {
			p1nfree(p);
			return l;
		}

		if ((p->n_type & TMASK) == 0 && (l->n_type & TMASK) == 0 &&
		    tsize(p->n_type, p->n_df, p->n_ap) ==
		    tsize(l->n_type, l->n_df, l->n_ap)) {
			if (p->n_type != FLOAT && p->n_type != DOUBLE &&
			    l->n_type != FLOAT && l->n_type != DOUBLE &&
			    l->n_type != LDOUBLE && p->n_type != LDOUBLE) {
				if (l->n_op == NAME || l->n_op == UMUL ||
				    l->n_op == TEMP) {
					l->n_type = p->n_type;
					p1nfree(p);
					return l;
				}
			}
		}

		if (DEUNSIGN(p->n_type) == INT && DEUNSIGN(l->n_type) == INT &&
		    coptype(l->n_op) == BITYPE && l->n_op != COMOP &&
		    l->n_op != QUEST && l->n_op != ASSIGN && l->n_op != RS) {
			l->n_type = p->n_type;
			p1nfree(p);
			return l;
		}

		o = l->n_op;
		m = p->n_type;
		if (o == ICON) {
			/*
			 * Can only end up here if o is an address,
			 * and in that case the only compile-time conversion
			 * possible is to int.
			 */

			if (l->n_sp == 0) {
				p->n_type = UNSIGNED;
				concast(l, m);
			} else if (m != INT && m != UNSIGNED)
				break;
			l->n_type = m;
			l->n_ap = 0;
			p1nfree(p);
			return l;
		}

		if (p->n_type != UCHAR || l->n_type != CHAR ||
		    l->n_op != UMUL || (l->n_left->n_op != TEMP))
			break;
		l->n_type = UCHAR;
		MODTYPE(l->n_left->n_type, UCHAR);
		p = nfree(p);
		break;

#if 0
	case STASG: /* struct assignment, modify left */
		l = p->n_left;
		if (ISSOU(l->n_type))
			p->n_left = buildtree(ADDROF, l, NIL);
		break;
#endif

	case FORCE:
		/* put return value in return reg */
		p->n_op = ASSIGN;
		p->n_right = p->n_left;
		p->n_left = block(REG, NIL, NIL, p->n_type, 0, 0);
		p->n_left->n_rval = p->n_left->n_type == BOOL ? 
		    RETREG(CHAR) : RETREG(p->n_type);
		break;

	}
#ifdef PCC_DEBUG
	if (xdebug) {
		printf("clocal end: %p\n", p);
		fwalk(p, eprint, 0);
	}
#endif
	return(p);
}

#define IALLOC(sz)      (isinlining ? permalloc(sz) : tmpalloc(sz))
void
myp2tree(NODE *p)
{
	struct symtab *sp;

	if (p->n_op != FCON)
		return;

	sp = IALLOC(sizeof(struct symtab));
	sp->sclass = STATIC;
	sp->sap = 0;
	sp->slevel = 1; /* fake numeric label */
	sp->soffset = getlab();
	sp->sflags = 0;
	sp->stype = p->n_type;
	sp->squal = (CON >> TSHIFT);
	sp->sname = NULL;

	locctr(DATA, sp);
	defloc(sp);
	ninval(0, tsize(sp->stype, sp->sdf, sp->sap), p);

	p->n_op = NAME;
	slval(p, 0);
	p->n_sp = sp;
}

/*ARGSUSED*/
int
andable(NODE *p)
{
	return(1);	/* all names can have & taken on them */
}

/*
 * Return 1 if a variable of type type is OK to put in register.
 */
int
cisreg(TWORD t)
{
//	if (t == FLOAT || t == DOUBLE || t == LDOUBLE ||
//	    t == LONGLONG || t == ULONGLONG)
//		return 0; /* not yet */
	return 1;
}

/*
 * Allocate off bits on the stack.  p is a tree that when evaluated
 * is the multiply count for off, t is a storeable node where to write
 * the allocated address.
 */
void
spalloc(NODE *t, NODE *p, OFFSZ off)
{
	NODE *sp;

	p = buildtree(MUL, p, bcon(off/SZCHAR)); /* XXX word alignment? */

	/* sub the size from sp */
	sp = block(REG, NIL, NIL, p->n_type, 0, 0);
	slval(sp, 0);
	sp->n_rval = STKREG;
	ecomp(buildtree(MINUSEQ, sp, p));

	/* save the address of sp */
	sp = block(REG, NIL, NIL, PTR+INT, t->n_df, t->n_ap);
	slval(sp, 0);
	sp->n_rval = STKREG;
	sp = buildtree(PLUS, sp, bcon(1));
	t->n_type = sp->n_type;
	ecomp(buildtree(ASSIGN, t, sp)); /* Emit! */

}

/*
 * Print out a string of characters.
 * Assume that the assembler understands C-style escape
 * sequences.
 */
void
instring(struct symtab *sp)
{
	unsigned short sh[2];
	int val, cnt;
	TWORD t;
	char *s;

	defloc(sp);
	t = BTYPE(sp->stype);
	s = sp->sname;
	if (t == UNSIGNED) {
		/* convert to UTF-16 */
		while (*s) {
			cp2u16(u82cp(&s), sh);
			if (sh[0]) printf("%o\n", sh[0]);
			if (sh[1]) printf("%o\n", sh[1]);
		}
		printf("0\n");
	} else if (t == CHAR) {
		for (cnt = 0; *s != 0; ) {
			if (cnt++ == 0)
				printf(".byte ");
			if (*s == '\\')
				val = esccon(&s);
			else
				val = *s++;
			printf("%o", val & 0377);
			if (cnt > 15) {
				cnt = 0;
				printf("\n");
			} else
				printf(",");
		}
		printf("%s0\n", cnt ? "" : ".byte ");
	} else
		cerror("instring");
}

/*
 * print out a constant node, may be associated with a label.
 * Do not free the node after use.
 * off is bit offset from the beginning of the aggregate
 * fsz is the number of bits this is referring to
 */
int
ninval(CONSZ off, int fsz, NODE *p)
{
#ifndef LANG_CXX
	SFP sfp = p->n_scon;
#endif
	TWORD t;
	int i;

	t = p->n_type;
	switch (t) {
	case LONGLONG:
	case ULONGLONG:
		i = (glval(p) >> 32);
		slval(p, glval(p) & 0xffffffff);
		p->n_type = INT;
		ninval(off, 32, p);
		slval(p, i);
		ninval(off+32, 32, p);
		break;
	case LONG:
	case ULONG:
		printf("%o ; %o\n", (int)((glval(p) >> 16) & 0177777),
		    (int)(glval(p) & 0177777));
		break;
#ifndef LANG_CXX
	case FLOAT:
		printf("%o ; %o\n", sfp->fp[0] & 0xffff, sfp->fp[0] >> 16);
		break;
	case LDOUBLE:
	case DOUBLE:
		printf("%o ; %o ; %o ; %o\n", sfp->fp[0] & 0xffff,
		    sfp->fp[0] >> 16, sfp->fp[1] & 0xffff, sfp->fp[1] >> 16);
		break;
#endif
	default:
		return 0;
	}
	return 1;
}

/* make a name look like an external name in the local machine */
char *
exname(char *p)
{
#define NCHNAM  256
	static char text[NCHNAM+1];
	int i;

	if (p == NULL)
		return "";

	text[0] = '_';
	for (i=1; *p && i<NCHNAM; ++i)
		text[i] = *p++;

	text[i] = '\0';
	text[NCHNAM] = '\0';  /* truncate */

	return (text);

}

/*
 * map types which are not defined on the local machine
 */
TWORD
ctype(TWORD type)
{
	switch (BTYPE(type)) {
	case SHORT:
		MODTYPE(type,INT);
		break;

	case USHORT:
		MODTYPE(type,UNSIGNED);
		break;

	case LDOUBLE:
		MODTYPE(type,DOUBLE);
		break;

	/* XXX remove as soon as 64-bit is added */
	case LONGLONG:
		MODTYPE(type,LONG);
		break;
	case ULONGLONG:
		MODTYPE(type,ULONG);
		break;
	}
	return (type);
}

void
calldec(NODE *p, NODE *q) 
{
}

void
extdec(struct symtab *q)
{
}

/* make a common declaration for id, if reasonable */
void
defzero(struct symtab *sp)
{
	extern int lastloc;
	char *n;
	int off;

	off = tsize(sp->stype, sp->sdf, sp->sap);
	off = (off+(SZCHAR-1))/SZCHAR;
	n = getexname(sp);
	if (sp->sclass == STATIC) {
		printf(".bss\n");
		if (sp->slevel == 0)
			printf("%s:", n);
		else
			printf(LABFMT ":", sp->soffset);
		printf("	.=.+%o\n", off);
		lastloc = -1;
		return;
	}
	printf(".comm ");
	if (sp->slevel == 0)
		printf("%s,0%o\n", n, off);
	else
		printf(LABFMT ",0%o\n", sp->soffset, off);
}

/*
 * Give target the opportunity of handling pragmas.
 */
int
mypragma(char *str)
{
	return 0;
}

/*
 * Called when a identifier has been declared.
 */
void
fixdef(struct symtab *sp)
{
}

void
pass1_lastchance(struct interpass *ip)
{
}
