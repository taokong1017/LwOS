#include <cpu.h>
#include <menuconfig.h>
#include <offsets.h>
#include <general.h>

.globl __start
.type __start, function
.section .init.text, "ax"
__start:
	/* Mask all exceptions */
	msr daifset, DAIFSET_ALL_BITS
	mrs x0, CurrentEL
	lsr x0, x0, MODE_EL_SHIFT
	and x0, x0, MODE_EL_MASK
	cmp x0, MODE_EL3
	b.eq el3_entry
	cmp x0, MODE_EL2
	b.eq el2_entry
	cmp x0, MODE_EL1
	b.eq el1_entry

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
	adr x0, el2_entry
	msr elr_el2, x0
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
	mov x0, (SPSR_DAIF_MASK | SPSR_MODE_EL1T)
	msr spsr_el2, x0

	/* clear cnt register for EL2 */
	msr cntvoff_el2, xzr
	msr cnthctl_el2, xzr

	/* set el1_entry */
	adr x0, el1_entry
	msr elr_el2, x0
	eret

el1_entry:
	/* check mpid is valid */
	ldr x0, =arch_boot_params
	ldr x1, [x0, #BOOT_PARAMS_OF_MP_ID_OFFSET]
	cmn x1, #1
	str x1, [x0, #BOOT_PARAMS_OF_MP_ID_OFFSET]
	b.eq primary_core

secondary_core:
	/* disable fp trap */
	mov x0, CPACR_EL1_FPEN_NOTRAP
	msr cpacr_el1, x0

	/* set stack pointer */
	.extern __kernel_stack_start
	ldr x0, =__kernel_stack_start
	ldr x1, =arch_boot_params
	mov x2, xzr
	dsb osh
	ldr w2, [x1, #BOOT_PARAMS_OF_CPU_ID_OFFSET]
	add x2, x2, #1
	mov x1, CONFIG_KERNEL_STACK_SIZE
	mul x3, x1, x2
	add x3, x0, x3
	msr spsel, #0
	mov sp, x3
	b arch_secondary_cpu_init

primary_core:
	/* disable instructions trap */
	mov x0, CPACR_EL1_FPEN_NOTRAP
	msr cpacr_el1, x0

	/* save master pmid */
	mrs x1, mpidr_el1
	ubfx x1, x1, #0, #23
	ldr x2, =cpu_master_mp_id
	str x1, [x2]

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
	.extern __kernel_stack_start
	ldr x0, =__kernel_stack_start
	ldr x1, =arch_boot_params
	dsb osh
	ldr w2, [x1, #BOOT_PARAMS_OF_CPU_ID_OFFSET]
	add x2, x2, #1
	mov x1, CONFIG_KERNEL_STACK_SIZE
	mul x3, x1, x2
	add x3, x0, x3
	msr spsel, #0
	mov sp, x3

	/* set up kernel memory map */
	bl kernel_mem_domain_init

	/* init uart */
	bl uart_early_init

	/* enable interrupt & exception */
	bl exec_enable

	/* enter the kernel entry */
	b kernel_start
