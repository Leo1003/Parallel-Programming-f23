	.text
	.file	"test1.cpp"
                                        # Start of file scope inline assembly
	.globl	_ZSt21ios_base_library_initv

                                        # End of file scope inline assembly
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function _Z5test1PfS_S_i
.LCPI0_0:
	.quad	0x3e112e0be826d695              # double 1.0000000000000001E-9
	.text
	.globl	_Z5test1PfS_S_i
	.p2align	4, 0x90
	.type	_Z5test1PfS_S_i,@function
_Z5test1PfS_S_i:                        # @_Z5test1PfS_S_i
	.cfi_startproc
# %bb.0:
	pushq	%r15
	.cfi_def_cfa_offset 16
	pushq	%r14
	.cfi_def_cfa_offset 24
	pushq	%r13
	.cfi_def_cfa_offset 32
	pushq	%r12
	.cfi_def_cfa_offset 40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	subq	$32, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -48
	.cfi_offset %r12, -40
	.cfi_offset %r13, -32
	.cfi_offset %r14, -24
	.cfi_offset %r15, -16
	movq	%rdx, %rbx
	movq	%rsi, %r14
	movq	%rdi, %r15
	movq	%fs:40, %rax
	movq	%rax, 24(%rsp)
	leaq	8(%rsp), %rsi
	movl	$1, %edi
	callq	clock_gettime@PLT
	testl	%eax, %eax
	jne	.LBB0_10
# %bb.1:
	movq	8(%rsp), %r13
	movq	16(%rsp), %r12
	movq	%rbx, %rax
	subq	%r15, %rax
	cmpq	$32, %rax
	setb	%cl
	movq	%rbx, %rax
	subq	%r14, %rax
	cmpq	$32, %rax
	setb	%al
	orb	%cl, %al
	xorl	%ecx, %ecx
	jmp	.LBB0_2
	.p2align	4, 0x90
.LBB0_4:                                #   in Loop: Header=BB0_2 Depth=1
	incl	%ecx
	cmpl	$20000000, %ecx                 # imm = 0x1312D00
	je	.LBB0_5
.LBB0_2:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_3 Depth 2
                                        #     Child Loop BB0_8 Depth 2
	xorl	%edx, %edx
	testb	%al, %al
	je	.LBB0_3
	.p2align	4, 0x90
.LBB0_8:                                #   Parent Loop BB0_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movss	(%r15,%rdx,4), %xmm0            # xmm0 = mem[0],zero,zero,zero
	addss	(%r14,%rdx,4), %xmm0
	movss	%xmm0, (%rbx,%rdx,4)
	movss	4(%r15,%rdx,4), %xmm0           # xmm0 = mem[0],zero,zero,zero
	addss	4(%r14,%rdx,4), %xmm0
	movss	%xmm0, 4(%rbx,%rdx,4)
	movss	8(%r15,%rdx,4), %xmm0           # xmm0 = mem[0],zero,zero,zero
	addss	8(%r14,%rdx,4), %xmm0
	movss	%xmm0, 8(%rbx,%rdx,4)
	movss	12(%r15,%rdx,4), %xmm0          # xmm0 = mem[0],zero,zero,zero
	addss	12(%r14,%rdx,4), %xmm0
	movss	%xmm0, 12(%rbx,%rdx,4)
	addq	$4, %rdx
	cmpq	$1024, %rdx                     # imm = 0x400
	jne	.LBB0_8
	jmp	.LBB0_4
	.p2align	4, 0x90
.LBB0_3:                                #   Parent Loop BB0_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movups	(%r15,%rdx,4), %xmm0
	movups	16(%r15,%rdx,4), %xmm1
	movups	(%r14,%rdx,4), %xmm2
	addps	%xmm0, %xmm2
	movups	16(%r14,%rdx,4), %xmm0
	addps	%xmm1, %xmm0
	movups	%xmm2, (%rbx,%rdx,4)
	movups	%xmm0, 16(%rbx,%rdx,4)
	movups	32(%r15,%rdx,4), %xmm0
	movups	48(%r15,%rdx,4), %xmm1
	movups	32(%r14,%rdx,4), %xmm2
	addps	%xmm0, %xmm2
	movups	48(%r14,%rdx,4), %xmm0
	addps	%xmm1, %xmm0
	movups	%xmm2, 32(%rbx,%rdx,4)
	movups	%xmm0, 48(%rbx,%rdx,4)
	addq	$16, %rdx
	cmpq	$1024, %rdx                     # imm = 0x400
	jne	.LBB0_3
	jmp	.LBB0_4
.LBB0_5:
	leaq	8(%rsp), %rsi
	movl	$1, %edi
	callq	clock_gettime@PLT
	testl	%eax, %eax
	jne	.LBB0_10
# %bb.6:
	movq	8(%rsp), %rax
	subq	%r13, %rax
	movq	16(%rsp), %rcx
	subq	%r12, %rcx
	xorps	%xmm0, %xmm0
	cvtsi2sd	%rax, %xmm0
	xorps	%xmm1, %xmm1
	cvtsi2sd	%rcx, %xmm1
	mulsd	.LCPI0_0(%rip), %xmm1
	addsd	%xmm0, %xmm1
	movsd	%xmm1, (%rsp)                   # 8-byte Spill
	movq	_ZSt4cout@GOTPCREL(%rip), %rbx
	leaq	.L.str(%rip), %rsi
	movl	$47, %edx
	movq	%rbx, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movq	%rbx, %rdi
	movsd	(%rsp), %xmm0                   # 8-byte Reload
                                        # xmm0 = mem[0],zero
	callq	_ZNSo9_M_insertIdEERSoT_@PLT
	movq	%rax, %rbx
	leaq	.L.str.1(%rip), %rsi
	movl	$8, %edx
	movq	%rax, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movq	%rbx, %rdi
	movl	$1024, %esi                     # imm = 0x400
	callq	_ZNSolsEi@PLT
	movq	%rax, %rbx
	leaq	.L.str.2(%rip), %rsi
	movl	$5, %edx
	movq	%rax, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movq	%rbx, %rdi
	movl	$20000000, %esi                 # imm = 0x1312D00
	callq	_ZNSolsEi@PLT
	leaq	.L.str.3(%rip), %rsi
	movl	$2, %edx
	movq	%rax, %rdi
	callq	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l@PLT
	movq	%fs:40, %rax
	cmpq	24(%rsp), %rax
	jne	.LBB0_9
# %bb.7:
	addq	$32, %rsp
	.cfi_def_cfa_offset 48
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	retq
.LBB0_10:
	.cfi_def_cfa_offset 80
	leaq	.L.str.4(%rip), %rdi
	leaq	.L.str.5(%rip), %rsi
	leaq	.L__PRETTY_FUNCTION__._ZL7gettimev(%rip), %rcx
	movl	$75, %edx
	callq	__assert_fail@PLT
.LBB0_9:
	callq	__stack_chk_fail@PLT
.Lfunc_end0:
	.size	_Z5test1PfS_S_i, .Lfunc_end0-_Z5test1PfS_S_i
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"Elapsed execution time of the loop in test1():\n"
	.size	.L.str, 48

	.type	.L.str.1,@object                # @.str.1
.L.str.1:
	.asciz	"sec (N: "
	.size	.L.str.1, 9

	.type	.L.str.2,@object                # @.str.2
.L.str.2:
	.asciz	", I: "
	.size	.L.str.2, 6

	.type	.L.str.3,@object                # @.str.3
.L.str.3:
	.asciz	")\n"
	.size	.L.str.3, 3

	.type	.L.str.4,@object                # @.str.4
.L.str.4:
	.asciz	"r == 0"
	.size	.L.str.4, 7

	.type	.L.str.5,@object                # @.str.5
.L.str.5:
	.asciz	"./fasttime.h"
	.size	.L.str.5, 13

	.type	.L__PRETTY_FUNCTION__._ZL7gettimev,@object # @__PRETTY_FUNCTION__._ZL7gettimev
.L__PRETTY_FUNCTION__._ZL7gettimev:
	.asciz	"fasttime_t gettime()"
	.size	.L__PRETTY_FUNCTION__._ZL7gettimev, 21

	.ident	"clang version 16.0.6"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym __stack_chk_fail
	.addrsig_sym _ZSt4cout
