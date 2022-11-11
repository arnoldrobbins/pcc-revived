	.section .init
	.globl _init
	.p2align 2
_init:
	link.w	%fp,#0
	.previous

	.section .fini
	.globl _fini
	.p2align 2
_fini:
	link.w	%fp,#0
	.previous
