	.extern __udivsi3

	| int __divsi3(int a, int b)
	.p2align 2
	.globl	__divsi3
__divsi3:
	move.l	%d2,-(%sp)
	move.l	#1,%d2		| sign of result stored in d2 (=1 or =-1)
	move.l	12(%sp),%d1	| d1 = divisor
	bpl	1f
	neg.l	%d1
	neg.l	%d2		| change sign because divisor <0
1:	move.l	8(%sp),%d0	| d0 = dividend
	bpl	2f
	neg.l	%d0
	neg.l	%d2
2:	move.l	%d1,-(%sp)
	move.l	%d0,-(%sp)
	bsr	__udivsi3	| divide abs(dividend) by abs(divisor)
	add.l	#8,%sp
	tst.b	%d2
	bpl	3f
	neg.l	%d0
3:	move.l	(%sp)+,%d2
	rts
