#include <offsets.h>

.macro SMCCC, instr
	stp x29, x30, [sp, #-16]!
	mov x29, sp
	\instr #0
	ldr x4, [sp, #16]
	stp x0, x1, [x4, ARCH_SMMCC_A0_A1_OFFSET]
	stp x2, x3, [x4, ARCH_SMMCC_A2_A3_OFFSET]
	stp x4, x5, [x4, ARCH_SMMCC_A4_A5_OFFSET]
	stp x6, x7, [x4, ARCH_SMMCC_A6_A7_OFFSET]
	ldp x29, x30, [sp], #16
	ret
.endm

.text
.globl arch_smccc_smc
.type arch_smccc_smc, function
.section .text, "ax"
arch_smccc_smc:
	SMCCC smc

.text
.globl arch_smccc_hvc
.type arch_smccc_hvc, function
.section .text, "ax"
arch_smccc_hvc:
	SMCCC hvc
