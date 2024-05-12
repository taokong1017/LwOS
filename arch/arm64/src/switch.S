#include <task_offsets.h>
#include <menuconfig.h>

/*
 * void arch_context_switch(arch_task_context_t *new, arch_task_context_t *old)
 */
.globl arch_context_switch
.type arch_context_switch, function
.section .text, "ax"
arch_context_switch:
	stp	x19, x20, [x1, #ARCH_TASK_CONTEXT_X19_X20_OFFSET]
	stp	x21, x22, [x1, #ARCH_TASK_CONTEXT_X21_X22_OFFSET]
	stp	x23, x24, [x1, #ARCH_TASK_CONTEXT_X23_X24_OFFSET]
	stp	x25, x26, [x1, #ARCH_TASK_CONTEXT_X25_X26_OFFSET]
	stp	x27, x28, [x1, #ARCH_TASK_CONTEXT_X27_X28_OFFSET]
	stp	x29, x30, [x1, #ARCH_TASK_CONTEXT_FP_LR_OFFSET]    /* save fp and lr */
	mov x3, sp
	str	x3, [x1, #ARCH_TASK_CONTEXT_SP_OFFSET]
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
	ldp	x29, x30, [x0, #ARCH_TASK_CONTEXT_FP_LR_OFFSET]
	str	x3, [x0, #ARCH_TASK_CONTEXT_SP_OFFSET]
	mov sp, x3
	ret
