|      $Id: crt0.s,v 1.2 2022/11/05 02:06:08 gmcgarry Exp $
|
| Copyright (c) 2022 Gregory McGarry <g.mcgarry@ieee.org>
|
| Permission to use, copy, modify, and distribute this software for any
| purpose with or without fee is hereby granted, provided that the above
| copyright notice and this permission notice appear in all copies.
|
| THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
| WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
| MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
| ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
| WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
| ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
| OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

	.extern	main

	.extern	__bss_start
	.extern	__bss_end
	.extern	__stack_top
	.extern _init
	.extern _fini
	.extern exit
	.extern atexit

	.global	_start
	.global	_default_handler
	.global	_exception

	.section .vtable
_vtable: 
	.long	__stack_top
	.long	_start
	.long	_exception	|  2: bus error
	.long	_exception	|  3: address error
	.long	_exception	|  4: illegal instruction
	.long	_exception	|  5: zero divide
	.long	_exception	|  6: chk
	.long	_exception	|  7: trapv
	.long	_exception	|  8: privilege violation
	.long	_exception	|  9: trace
	.long	_exception	| 10: 1010
	.long	_exception	| 11: 1111
	.long	_exception	| 12: -
	.long	_exception	| 13: -
	.long	_exception	| 14: -
	.long	_exception	| 15: uninitialized interrupt
	.long	_exception	| 16: -
	.long	_exception	| 17: -
	.long	_exception	| 18: -
	.long	_exception	| 19: -
	.long	_exception	| 20: -
	.long	_exception	| 21: -
	.long	_exception	| 22: -
	.long	_exception	| 23: -
	.long	_default_handler| 24: spurious interrupt
	.long	_default_handler| 25: l1 irq
	.long	_default_handler| 26: l2 irq
	.long	_default_handler| 27: l3 irq
	.long	_default_handler| 28: l4 irq
	.long	_default_handler| 29: l5 irq
	.long	_default_handler| 30: l6 irq
	.long	_default_handler| 31: l7 irq
	.long	_exception	| 32: trap 0
	.long	_exception	| 33: trap 1
	.long	_exception	| 34: trap 2
	.long	_exception	| 35: trap 3
	.long	_exception	| 36: trap 4
	.long	_exception	| 37: trap 5
	.long	_exception	| 38: trap 6
	.long	_exception	| 39: trap 7
	.long	_exception	| 40: trap 8
	.long	_exception	| 41: trap 9
	.long	_exception	| 42: trap 10
	.long	_exception	| 43: trap 11
	.long	_exception	| 44: trap 12
	.long	_exception	| 45: trap 13
	.long	_exception	| 46: trap 14
	.long	_exception	| 47: trap 15
	| This is the end of the useful part of the table.

_default_handler:
	rte

_exception:
	bra	.

	.section .text
_start:
 	move.l	#__bss_start,%a0
	move.l	#__bss_end,%a1
	move.l	%a1,%d1
	sub.l	%a0,%d1
	beq	2f
	clr.l 	%d0
1:	move.l	%d0,(%a0)+
	sub.l	#1,%d1
	bgt	1b
2:
	jsr	_init
	pea.l	_fini
	jsr	atexit
	pea.l	0.l		| argv=NULL
	pea.l	0.l		| argc=0
	jsr	main 
	jmp	exit
	bra	.	

	.end
