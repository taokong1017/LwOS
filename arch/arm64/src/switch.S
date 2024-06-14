#include <cpu.h>
#include <task_offsets.h>
#include <menuconfig.h>

/*
 * void arch_task_context_switch(arch_task_context_t *new, arch_task_context_t *old)
 */
.globl arch_task_context_switch
.type arch_task_context_switch, function
.section .text, "ax"
arch_task_context_switch:
	mrs x2, sp_el0
	mov	x3, sp
	stp	x19, x20, [x1, #ARCH_TASK_CONTEXT_X19_X20_OFFSET]
	stp	x21, x22, [x1, #ARCH_TASK_CONTEXT_X21_X22_OFFSET]
	stp	x23, x24, [x1, #ARCH_TASK_CONTEXT_X23_X24_OFFSET]
	stp	x25, x26, [x1, #ARCH_TASK_CONTEXT_X25_X26_OFFSET]
	stp	x27, x28, [x1, #ARCH_TASK_CONTEXT_X27_X28_OFFSET]
	stp	x29, x2,  [x1, #CALLEE_CONTEXT_OF_X29_SP_EL0_OFFSET]
	stp	x3,  lr,  [x1, #CALLEE_CONTEXT_OF_SP_ELx_LR_OFFSET]
#ifdef CONFIG_FPU_ENABLE
	stp x0, x1, [sp, #-16]!
	add x0, x1, #ARCH_TASK_FP_CONTEXT_OFFSET
	bl arch_fpu_save
	ldp x0, x1, [sp], #-16
	stp x0, x1, [sp, #-16]!
	add x0, x0, #ARCH_TASK_FP_CONTEXT_OFFSET
	bl arch_fpu_restore
	ldp x0, x1, [sp], #-16
#endif
	ldp	x19, x20, [x0, #ARCH_TASK_CONTEXT_X19_X20_OFFSET]
	ldp	x21, x22, [x0, #ARCH_TASK_CONTEXT_X21_X22_OFFSET]
	ldp	x23, x24, [x0, #ARCH_TASK_CONTEXT_X23_X24_OFFSET]
	ldp	x25, x26, [x0, #ARCH_TASK_CONTEXT_X25_X26_OFFSET]
	ldp	x27, x28, [x0, #ARCH_TASK_CONTEXT_X27_X28_OFFSET]
	ldp	x29, x2,  [x0, #CALLEE_CONTEXT_OF_X29_SP_EL0_OFFSET]
	ldp	x3,  lr,  [x0, #CALLEE_CONTEXT_OF_SP_ELx_LR_OFFSET]
	msr sp_el0, x2
	mov	sp, x3
	ret

/*
 * void arch_main_task_switch(struct task *task)
 */
.globl arch_main_task_switch
.type arch_main_task_switch, function
.section .text, "ax"
arch_main_task_switch:
	/* switch to new task's sp */
	ldr	x1, [x0, #TASK_OF_STACK_PTR_OFFSET]
	mov	sp, x1

	msr daifclr, DAIFCLR_ALL_BITS
	ldr	x4, [x0, #TASK_OF_ENTRY_OFFSET]
	ldr	x3, [x0, #TASK_OF_ARGS_OFFSET + 24]
	ldr	x2, [x0, #TASK_OF_ARGS_OFFSET + 16]
	ldr	x1, [x0, #TASK_OF_ARGS_OFFSET + 8]
	ldr	x0, [x0, #TASK_OF_ARGS_OFFSET]
	blr x4
	ret

/*
 * void arch_exc_exit()
 */
.globl arch_exc_exit
.type arch_exc_exit, function
.section .text, "ax"
arch_exc_exit:
	ldp	x0, x1, [sp, #ESF_CONTEXT_OF_SPSR_ELR_OFFSET]
	msr	spsr_el1, x0
	msr	elr_el1, x1
	ldp	x0, x1, [sp, #ESF_CONTEXT_OF_X0_X1_OFFSET]
	ldp	x2, x3, [sp, #ESF_CONTEXT_OF_X2_X3_OFFSET]
	ldp	x4, x5, [sp, #ESF_CONTEXT_OF_X4_X5_OFFSET]
	ldp	x6, x7, [sp, #ESF_CONTEXT_OF_X6_X7_OFFSET]
	ldp	x8, x9, [sp, #ESF_CONTEXT_OF_X8_X9_OFFSET]
	ldp	x10, x11, [sp, #ESF_CONTEXT_OF_X10_X11_OFFSET]
	ldp	x12, x13, [sp, #ESF_CONTEXT_OF_X12_X13_OFFSET]
	ldp	x14, x15, [sp, #ESF_CONTEXT_OF_X14_X15_OFFSET]
	ldp	x16, x17, [sp, #ESF_CONTEXT_OF_X16_X17_OFFSET]
	ldp	x18, lr,  [sp, #ESF_CONTEXT_OF_X18_LR_OFFSET]
	add	sp, sp, #ESF_CONTEXT_SIZE
	eret
