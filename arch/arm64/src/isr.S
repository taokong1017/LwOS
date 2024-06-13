#include <menuconfig.h>
#include <irq.h>

.globl isr_func
.type isr_func, function
.section .text, "ax"
isr_func:
	/* save LR register */
	str x30, [sp, #-8]!

	/* check if this irq is a spurious interrupt */
	bl arm_gic_get_active
	mov	x1, #IRQ_MAX_NUM - 1
	cmp	x0, x1
	b.hi spurious_continue

	stp	x0, xzr, [sp, #-16]!
	/* retrieve the interrupt service routine */
	ldr	x1, =sw_isr_table
	add	x1, x1, x0, lsl #4
	ldp	x0, x3, [x1]

	/* call the ISR, and mask interrupts, later surport unmask */
	blr	x3

	/* signal end-of-interrupt */
	ldp	x0, xzr, [sp], #16

spurious_continue:
	/* end of interrupt */
	bl	arm_gic_eoi

	/* reschedule task */
	bl task_reschedule_irq

	/* store LR register */
	ldr	x30, [sp], #8
	ret
