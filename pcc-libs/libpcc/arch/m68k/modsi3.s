	.extern __udivsi3
	.extern __mulsi3

	| unsigned int __modsi3(int a, int b)
	.p2align 2
	.globl	__modsi3
__modsi3:
	move.l	8(%sp),%d1	| d1 = divisor
	move.l	4(%sp),%d0	| d0 = dividend
	move.l	%d1,-(%sp)
	move.l	%d0,-(%sp)
	bsr	__divsi3
	add.l	#8,%sp
	move.l	8(%sp),%d1	| d1 = divisor
	move.l	%d1,-(%sp)
	move.l	%d0,-(%sp)
	bsr	__mulsi3	| d0 = (a/b)*b
	add.l	#8,%sp
	move.l	4(%sp),%d1	| d1 = dividend
	sub.l	%d0,%d1		| d1 = a - (a/b)*b
	move.l	%d1,%d0
	rts
