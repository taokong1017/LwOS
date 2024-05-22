#include <cpu.h>

.globl __start
.type __start, function
.section .init.text, "ax"
__start:
	/* Mask all exceptions */
	msr	DAIFSet, #0xf
	mrs	x0, CurrentEL
	lsr x0, x0, MODE_EL_SHIFT
	and x0, x0, MODE_EL_MASK
	cmp	x0, MODE_EL3
	beq	el3_entry
	cmp	x0, MODE_EL2
	beq	el2_entry
	cmp	x0, MODE_EL1
	beq	el1_entry

el3_entry:
	/* set cptr_el3 register */
	mrs x0, cptr_el3
	bic x0, x0, CPTR_TTA_BIT   /* Do not trap sysreg accesses */
	bic x0, x0, CPTR_TFP_BIT   /* Do not trap SVE, SIMD and FP */
	bic x0, x0, CPTR_TCPAC_BIT /* Do not trap CPTR_EL2 / CPACR_EL1 accesses */
	msr cptr_el3, x0

	/* set scr_el3 register */
	mrs x0, scr_el3
	orr x0, x0, SCR_RW_BIT
	orr x0, x0, SCR_ST_BIT
	orr x0, x0, SCR_HCE_BIT
	orr x0, x0, SCR_SMD_BIT
	msr scr_el3, x0

	/* set el2_entry */
	adr	x0, el2_entry
	msr	elr_el2, x0
	eret

el2_entry:
	/* set sctlr_el2 register */
	mrs x0, sctlr_el2
	orr x0, x0, SCTLR_I_BIT
	orr x0, x0, SCTLR_SA_BIT
	msr sctlr_el2, x0

	/* set hcr_el2 register */
	mrs x0, hcr_el2
	bic x0, x0, HCR_IMO_BIT
	bic x0, x0, HCR_AMO_BIT
	bic x0, x0, HCR_TGE_BIT
	orr x0, x0, HCR_RW_BIT
	msr hcr_el2, x0

	/* set cptr_el2 register */
	mrs x0, cptr_el2
	bic x0, x0, CPTR_TFP_BIT
	bic x0, x0, CPTR_TCPAC_BIT
	msr cptr_el2, x0

	/* set spsr_el2 register*/
	mov x0, SPSR_DAIF_MASK
	orr x0, x0, SPSR_MODE_EL1T
	msr	spsr_el2, x0

	/* clear cnt register for EL2 */
	msr cntvoff_el2, xzr
	msr cnthctl_el2, xzr

	/* set el1_entry */
	adr	x0, el1_entry
	msr	elr_el2, x0
	eret

el1_entry:
	/* use SP_ELx */
	msr	SPSel, #1

	/* disable instructions trap */
	mov x0, CPACR_EL1_FPEN_NOTRAP
	msr cpacr_el1, x0

	/* clear bss */
clear_bss:
	ldr x0, =__bss_start
	ldr x1, =__bss_end
	sub x1, x1, x0
	cbz x1, bss_loop_done
bss_loop:
	sub x1, x1, #8
	str xzr, [x0], #8
	cbnz x1, bss_loop
bss_loop_done:

	/* set stack pointer */
	.extern __exec_stack_end
	.extern __kernel_stack_end
	ldr x0, =__exec_stack_end
	msr SPSel, #1
	mov sp, x0
	ldr x0, =__kernel_stack_end
	msr SPSel, #0
	mov sp, x0

	/* set up early kernel memory map */
	bl early_kernel_map
	bl uart_early_init

	/* enable MMU */
	bl mmu_enable

	/* enable interrupt & exception */
	bl exec_enable

	b .
