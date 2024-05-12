/*
 * Four types of exceptions:
 * - synchronous: aborts from MMU, SP/CP alignment checking, unallocated
 *   instructions, SVCs/SMCs/HVCs, ...)
 * - IRQ: group 1 (normal) interrupts
 * - FIQ: group 0 or secure interrupts
 * - SError: fatal system errors
 *
 * Four different contexts:
 * - from same exception level, when using the SP_EL0 stack pointer
 * - from same exception level, when using the SP_ELx stack pointer
 * - from lower exception level, when this is AArch64
 * - from lower exception level, when this is AArch32
 *
 * +------------------+------------------+-------------------------+
 * |     Address      |  Exception type  |       Description       |
 * +------------------+------------------+-------------------------+
 * | VBAR_ELn + 0x000 | Synchronous      | Current EL with SP0     |
 * |          + 0x080 | IRQ / vIRQ       |                         |
 * |          + 0x100 | FIQ / vFIQ       |                         |
 * |          + 0x180 | SError / vSError |                         |
 * +------------------+------------------+-------------------------+
 * |          + 0x200 | Synchronous      | Current EL with SPx     |
 * |          + 0x280 | IRQ / vIRQ       |                         |
 * |          + 0x300 | FIQ / vFIQ       |                         |
 * |          + 0x380 | SError / vSError |                         |
 * +------------------+------------------+-------------------------+
 * |          + 0x400 | Synchronous      | Lower EL using AArch64  |
 * |          + 0x480 | IRQ / vIRQ       |                         |
 * |          + 0x500 | FIQ / vFIQ       |                         |
 * |          + 0x580 | SError / vSError |                         |
 * +------------------+------------------+-------------------------+
 * |          + 0x600 | Synchronous      | Lower EL using AArch32  |
 * |          + 0x680 | IRQ / vIRQ       |                         |
 * |          + 0x700 | FIQ / vFIQ       |                         |
 * |          + 0x780 | SError / vSError |                         |
 * +------------------+------------------+-------------------------+
 */
#include <cpu_defines.h>

.macro	ventry	label
	.align	7
	b	\label
.endm

.macro CALL_EXCEPTION_CFUNC cfunc
	mov	x0, sp
	bl	\cfunc
.endm

.macro EXCEPTION_HANDLER irqname
.align 7
\irqname:
.endm

/* Push registers on stack */
.macro	PUSH_GENERAL_REGS
	sub	sp, sp,   #ARM_ARCH_REGS_SIZE
	stp	x28, x29, [sp, #ARM_ARCH_REGS_GPR28]
	stp	x26, x27, [sp, #ARM_ARCH_REGS_GPR26]
	stp	x24, x25, [sp, #ARM_ARCH_REGS_GPR24]
	stp	x22, x23, [sp, #ARM_ARCH_REGS_GPR22]
	stp	x20, x21, [sp, #ARM_ARCH_REGS_GPR20]
	stp	x18, x19, [sp, #ARM_ARCH_REGS_GPR18]
	stp	x16, x17, [sp, #ARM_ARCH_REGS_GPR16]
	stp	x14, x15, [sp, #ARM_ARCH_REGS_GPR14]
	stp	x12, x13, [sp, #ARM_ARCH_REGS_GPR12]
	stp	x10, x11, [sp, #ARM_ARCH_REGS_GPR10]
	stp	x8,  x9,  [sp, #ARM_ARCH_REGS_GPR8]
	stp	x6,  x7,  [sp, #ARM_ARCH_REGS_GPR6]
	stp	x4,  x5,  [sp, #ARM_ARCH_REGS_GPR4]
	stp	x2,  x3,  [sp, #ARM_ARCH_REGS_GPR2]
	stp	x0,  x1,  [sp, #ARM_ARCH_REGS_GPR0]
	add	x29, sp,  #ARM_ARCH_REGS_SIZE
	stp	x30, x29, [sp, #ARM_ARCH_REGS_LR]
	mrs	x22, elr_el1
	mrs	x23, spsr_el1
	stp	x22, x23, [sp, #ARM_ARCH_REGS_PC]
	/*
	 * Registers that may be useful after this macro is invoked:
	 *
	 * x21 - aborted SP
	 * x22 - aborted PC
	 * x23 - aborted PSTATE
	 */
.endm

/* Pop registers from stack */
.macro	POP_GENERAL_REGS
	ldp	x22, x23, [sp, #ARM_ARCH_REGS_PC]
	msr	elr_el1,  x22
	msr	spsr_el1, x23
	ldp	x30, x28, [sp, #ARM_ARCH_REGS_LR]
	mov	x29, sp
	mov	sp,  x28
	ldp	x0,  x1,  [x29, #ARM_ARCH_REGS_GPR0]
	ldp	x2,  x3,  [x29, #ARM_ARCH_REGS_GPR2]
	ldp	x4,  x5,  [x29, #ARM_ARCH_REGS_GPR4]
	ldp	x6,  x7,  [x29, #ARM_ARCH_REGS_GPR6]
	ldp	x8,  x9,  [x29, #ARM_ARCH_REGS_GPR8]
	ldp	x10, x11, [x29, #ARM_ARCH_REGS_GPR10]
	ldp	x12, x13, [x29, #ARM_ARCH_REGS_GPR12]
	ldp	x14, x15, [x29, #ARM_ARCH_REGS_GPR14]
	ldp	x16, x17, [x29, #ARM_ARCH_REGS_GPR16]
	ldp	x18, x19, [x29, #ARM_ARCH_REGS_GPR18]
	ldp	x20, x21, [x29, #ARM_ARCH_REGS_GPR20]
	ldp	x22, x23, [x29, #ARM_ARCH_REGS_GPR22]
	ldp	x24, x25, [x29, #ARM_ARCH_REGS_GPR24]
	ldp	x26, x27, [x29, #ARM_ARCH_REGS_GPR26]
	ldp	x28, x29, [x29, #ARM_ARCH_REGS_GPR28]
	eret
.endm

EXCEPTION_HANDLER el1t_sync_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1t_irq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1t_firq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1t_serror_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1h_sync_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1h_irq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1h_firq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER el1h_serror_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch64_el0_sync_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch64_el0_irq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch64_el0_firq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch64_el0_serror_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch32_el0_sync_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch32_el0_irq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch32_el0_firq_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

EXCEPTION_HANDLER aarch32_el0_serror_handler
	PUSH_GENERAL_REGS
	CALL_EXCEPTION_CFUNC default_exception_handler
	POP_GENERAL_REGS

.globl _vector_table
.type arch_fpu_save, object
.section .entry.text, "ax"
_vector_table:
.align 11
	ventry el1t_sync_handler   /* Synchronous EL1t */
	ventry el1t_irq_handler    /* IRQ EL1t */
	ventry el1t_firq_handler   /* FIQ EL1t */
	ventry el1t_serror_handler /* Error EL1t */

	ventry el1h_sync_handler   /* Synchronous EL1h */
	ventry el1h_irq_handler    /* IRQ EL1h */
	ventry el1h_firq_handler   /* FIQ EL1h */
	ventry el1h_serror_handler /* Error EL1h */

	ventry aarch64_el0_sync_handler    /* Synchronous 64-bit EL0 */
	ventry aarch64_el0_irq_handler     /* IRQ 64-bit EL0 */
	ventry aarch64_el0_firq_handler    /* FIQ 64-bit EL0 */
	ventry aarch64_el0_serror_handler  /* Error 64-bit EL0 */

	ventry aarch32_el0_sync_handler    /* Synchronous 32-bit EL0 */
	ventry aarch32_el0_irq_handler     /* IRQ 32-bit EL0 */
	ventry aarch32_el0_firq_handler    /* FIQ 32-bit EL0 */
	ventry aarch32_el0_serror_handler  /* Error 32-bit EL0 */
