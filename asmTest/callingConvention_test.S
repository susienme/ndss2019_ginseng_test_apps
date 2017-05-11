	.file	"callingConvention.c"
	.global	g
	.data
	.align	3
	.type	g, %object
	.size	g, 8
g:
	.xword	123
	.section	.rodata
	.align	3
.LC0:
	.string	"foo#1 g(%lld) old_SP(0x%llx) old_FP(0x%llx)\n"
	.align	3
.LC1:
	.string	"foo#2 g(%lld) new_SP(0x%llx) new_FP(0x%llx)\n"
	.text
	.align	2
	.global	foo
	.arch armv8-a+fp+simd
	//.tune generic
	.type	foo, %function
foo:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	str	x0, [x29, 24]
	str	x1, [x29, 16]

	// OLD
	adrp	x0, g
	add	x0, x0, :lo12:g
	ldr	x1, [x0]
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	ldr	x3, [x29, 16]
	ldr	x2, [x29, 24]
	bl	printf

	// NEW
	adrp	x0, g
	add	x0, x0, :lo12:g
	ldr	x1, [x0]
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	add	x2, sp, 32		// old sp
	ldr	x3, [sp]		// old FP
	bl	printf

	mov	x0, 99
	ldp	x29, x30, [sp], 32
	ret
	.size	foo, .-foo
	.section	.rodata
	.align	3
.LC2:
	.string	"bar g(%lld)\n"
	.text
	.align	2
	.global	bar
	.arch armv8-a+fp+simd
	//.tune generic
	.type	bar, %function
bar:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	adrp	x0, g
	add	x0, x0, :lo12:g
	ldr	x1, [x0]
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	printf
	nop
	ldp	x29, x30, [sp], 16
	ret
	.size	bar, .-bar
	.align	2
	.global	printStack
	.arch armv8-a+fp+simd
	//.tune generic
	.type	printStack, %function
printStack:
	sub	sp, sp, #16
	str	x0, [sp, 8]
	str	w1, [sp, 4]
	nop
	add	sp, sp, 16
	ret
	.size	printStack, .-printStack
	.section	.rodata
	.align	3
.LC3:
	.string	"ARGC (%d)\n"
	.text
	.align	2
	.global	main
	.arch armv8-a+fp+simd
	//.tune generic
	.type	main, %function
main:
	stp	x29, x30, [sp, -32]!

	//add	x29, sp, 0
	str	w0, [sp, 28]
	str	x1, [sp, 16]
	
	mov	x1, 500
	// mov	x0, 100
	//ldr x0, [sp, 8]
	mov x0, sp
	add x0, x0, 32	// old SP
	ldr x1, [sp]	// old fp

	//mov x0, sp 		// save NEW_SP to x0
	// x1 has old sp
	
	bl	foo
	bl	bar
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	ldr	w1, [sp, 28]
	bl	printf
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	ret
	.size	main, .-main
	.ident	"GCC: (Linaro GCC 5.3-2016.02) 5.3.1 20160113"
	.section	.note.GNU-stack,"",@progbits
