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
	stp	x0, x1,   [x1, #ARCH_TASK_CONTEXT_X0_X1_OFFSET]
	stp	x2, x3,   [x1, #ARCH_TASK_CONTEXT_X2_X3_OFFSET]
	stp	x4, x5,   [x1, #ARCH_TASK_CONTEXT_X4_X5_OFFSET]
	stp	x6, x7,   [x1, #ARCH_TASK_CONTEXT_X6_X7_OFFSET]
	stp	x8, x9,   [x1, #ARCH_TASK_CONTEXT_X8_X9_OFFSET]
	stp	x10, x11, [x1, #ARCH_TASK_CONTEXT_X10_X11_OFFSET]
	stp	x12, x13, [x1, #ARCH_TASK_CONTEXT_X12_X13_OFFSET]
	stp	x14, x15, [x1, #ARCH_TASK_CONTEXT_X14_X15_OFFSET]
	stp	x16, x17, [x1, #ARCH_TASK_CONTEXT_X16_X17_OFFSET]
	stp	x18, x19, [x1, #ARCH_TASK_CONTEXT_X18_X19_OFFSET]
	stp	x20, x21, [x1, #ARCH_TASK_CONTEXT_X20_X21_OFFSET]
	stp	x22, x23, [x1, #ARCH_TASK_CONTEXT_X22_X23_OFFSET]
	stp	x24, x25, [x1, #ARCH_TASK_CONTEXT_X24_X25_OFFSET]
	stp	x26, x27, [x1, #ARCH_TASK_CONTEXT_X26_X27_OFFSET]
	stp	x28, x29, [x1, #ARCH_TASK_CONTEXT_X28_X29_OFFSET]
	mrs x2, daif
	mrs x3, nzcv
	stp	x2, x3,  [x1, #ARCH_TASK_CONTEXT_DAIF_NZCV_OFFSET]
	mov x2, sp
	stp	x30, x2, [x1, #ARCH_TASK_CONTEXT_X30_SP_OFFSET]

#ifdef CONFIG_FPU_ENABLE
	stp x0, x1, [sp, #-16]!
	add x0, x1, #ARCH_TASK_FP_CONTEXT_OFFSET
	bl arch_fpu_save
	ldp x0, x1, [sp], #16
	stp x0, x1, [sp, #-16]!
	add x0, x0, #ARCH_TASK_FP_CONTEXT_OFFSET
	bl arch_fpu_restore
	ldp x0, x1, [sp], #16
#endif
	ldp	x30, x2,  [x0, #ARCH_TASK_CONTEXT_X30_SP_OFFSET]
	mov sp, x2
	ldp	x2, x3,   [x0, #ARCH_TASK_CONTEXT_DAIF_NZCV_OFFSET]
	msr daif, x2
	msr nzcv, x3
	ldp	x28, x29, [x0, #ARCH_TASK_CONTEXT_X28_X29_OFFSET]
	ldp	x26, x27, [x0, #ARCH_TASK_CONTEXT_X26_X27_OFFSET]
	ldp	x24, x25, [x0, #ARCH_TASK_CONTEXT_X24_X25_OFFSET]
	ldp	x22, x23, [x0, #ARCH_TASK_CONTEXT_X22_X23_OFFSET]
	ldp	x20, x21, [x0, #ARCH_TASK_CONTEXT_X20_X21_OFFSET]
	ldp	x18, x19, [x0, #ARCH_TASK_CONTEXT_X18_X19_OFFSET]
	ldp	x16, x17, [x0, #ARCH_TASK_CONTEXT_X16_X17_OFFSET]
	ldp	x14, x15, [x0, #ARCH_TASK_CONTEXT_X14_X15_OFFSET]
	ldp	x12, x13, [x0, #ARCH_TASK_CONTEXT_X12_X13_OFFSET]
	ldp	x10, x11, [x0, #ARCH_TASK_CONTEXT_X10_X11_OFFSET]
	ldp	x8, x9,   [x0, #ARCH_TASK_CONTEXT_X8_X9_OFFSET]
	ldp	x6, x7,   [x0, #ARCH_TASK_CONTEXT_X6_X7_OFFSET]
	ldp	x4, x5,   [x0, #ARCH_TASK_CONTEXT_X4_X5_OFFSET]
	ldp	x2, x3,   [x0, #ARCH_TASK_CONTEXT_X2_X3_OFFSET]
	ldp	x0, x1,   [x0, #ARCH_TASK_CONTEXT_X0_X1_OFFSET]
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

/*
 * uint64_t current_pc_get()
 */
.globl current_pc_get
.type current_pc_get, function
.section .text, "ax"
current_pc_get:
	mov x0, lr
	ret
