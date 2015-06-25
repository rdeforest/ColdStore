.text
	.align 4
.globl caller__FPFe_PvPvi
	.type	 caller__FPFe_PvPvi,@function
caller__FPFe_PvPvi:
	pushl	%ebp	# save frame
	movl	%esp,%ebp	# start new frame
	subl $12,%esp		# add some extra space to stack

	# save some regs
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	# get address
	call	.L1
.L1:	popl	%ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L1],%ebx

	# calculate arg space
	movl 20(%ebp),%eax	# get count

	# adjust stack for args, copy args to stack
	movl	16(%ebp),%ecx	# get array

	# push args
1:	subl	$1,%eax
	je	2f

	pushl	%ecx
	addl	$4,%ecx
	jmp 1b

2:	pushl	(%ecx)	# push `this'

	# push return value pointer
#	movl	8(%ebp), %esi
	push	%esi	

	movl 12(%ebp),%eax	# get fn
	call *%eax		# call the fn

	# move retval pointer to eax
	movl	10(%ebp), %eax

	# restore regs
	leal	-24(%ebp),%esp
	popl	%ebx
	popl	%esi
	popl	%edi
	leave
	ret
