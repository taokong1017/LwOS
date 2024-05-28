#include <menuconfig.h>
#include <irq.h>

.globl cpu_id_get
.type cpu_id_get, function
.section .text, "ax"
cpu_id_get:
	mrs x0, mpidr_el1
	ubfx x0, x0, #0, #24
	ret

.globl int_service_routine
.type int_service_routine, function
.section .text, "ax"
int_service_routine:
	/* check if this irq is a spurious interrupt */
	bl arm_gic_get_active
	mov	x1, #IRQ_MAX_NUM - 1
	cmp	x0, x1
	b.hi spurious_continue

	/* retrieve the interrupt service routine */
	ldr	x1, =sw_isr_table
	add	x1, x1, x0, lsl #4
	ldp	x0, x3, [x1]

	/* call the ISR, and mask interrupts, later surport unmask */
	blr	x3

spurious_continue:
	/* end of interrupt */
	bl	arm_gic_eoi
	ret