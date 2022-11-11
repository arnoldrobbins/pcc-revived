	| int __mulsi3(short a, short b)
	.p2align 2
	.globl __mulsi3
__mulsi3:
	move.w	4(%sp),%d0
	move.w	6(%sp),%d1
	mulu.w	10(%sp),%d0	| x0*y1
	mulu.w	8(%sp),%d1	| x1*y0
	add.l	%d1,%d0
	swap	%d0
	clr.w	%d0
	move.w	6(%sp),%d1
	mulu.w	10(%sp),%d1	| x1*y1
	add.l	%d1,%d0
	rts
