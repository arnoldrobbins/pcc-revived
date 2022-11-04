	| unsigned int __udivdi3(unsigned int a, unsigned int b)
	.p2align 2
	.globl	__udivsi3
__udivsi3:
	move.l	%d2,-(%sp)
	move.l	12(%sp),%d1	| d1 = divisor
	move.l	8(%sp),%d0	| d0 = dividend
	cmp.l	#0x10000,%d1	| divisor >= 2 ^ 16 ?
	bcc	3f		| then try next algorithm
	move.l	%d0,%d2
	clr.w	%d2
	swap	%d2
	divu	%d1,%d2		| high quotient in lower word 
	move.w	%d2,%d0		| save high quotient
	swap	%d0
	move.w	10(%sp),%d2	| get low dividend + high rest
	divu	%d1,%d2		| low quotient
	move.w	%d2,%d0
	bra	6f
3:	move.l	%d1,%d2		| use d2 as divisor backup
4:	lsr.l	#1,%d1		| shift divisor
	lsr.l	#1,%d0		| shift dividend
	cmp.l	#0x10000,%d1	| still divisor >= 2 ^ 16 ?
	bcc	4b
	divu	%d1,%d0		| now we have 16 bit divisor
	and.l	#0xffff,%d0	| mask out divisor, ignore remainder
	move.l	%d2,%d1
	mulu	%d0,%d1		| low part, 32 bits
	swap	%d2
	mulu	%d0,%d2		| high part, at most 17 bits
	swap	%d2		| align high part with low part
	tst.w	%d2		| high part 17 bits?
	bne	5f		| if 17 bits, quotient was too large
	add.l	%d2,%d1		| add parts
	bcs	5f		| if sum is 33 bits, quotient was too large
	cmp.l	8(%sp),%d1	| compare the sum with the dividend
	bls	6f		| if sum > dividend, quotient was too large
5:	sub.l	#1,%d0		| adjust quotient
6:	move.l	(%sp)+,%d2
	rts
