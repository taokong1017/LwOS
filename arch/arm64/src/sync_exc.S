#include <menuconfig.h>
#include <arch_syscall.h>
#include <offsets.h>
#include <cpu_defines.h>
#include <cpu.h>
#include <general.h>
#include <pgtable_hwdef.h>

.globl syscall_func
.type syscall_func, function
.section .text, "ax"
syscall_func:
	ldp x0, x1, [sp, #ARM_ARCH_REGS_GPR0]
	ldp x2, x3, [sp, #ARM_ARCH_REGS_GPR2]
	ldp x4, x5, [sp, #ARM_ARCH_REGS_GPR4]
	mov x6, x0

	ldr x8, [sp, ARM_ARCH_REGS_GPR8]
	bl  syscall_table_size_get
	sub x9, x0, #1
	mov x0, x6
	cmp x8, x9
	ble valid_syscall
	mov x8, x9

valid_syscall:
	ldr x9, =syscall_table
	ldr x9, [x9, x8, lsl #3]
	msr daifclr, DAIFCLR_IRQ_BIT
	blr x9
	msr daifset, DAIFSET_IRQ_BIT
	str x0, [sp, ARM_ARCH_REGS_GPR0]
	ret

.globl sync_func
.type sync_func, function
.section .text, "ax"
sync_func:
	mrs x0, esr_el1
	lsr x1, x0, #26
	cmp x1, #0x15 /* 0x15 = svc */
	bne inv

	and x1, x0, #0xff
	cmp x1, SVC_CALL_SYSTEM_CALL
	beq syscall_func
	ret

inv:
	b .

/*
 * int32_t buffer_validate(const void *addr, size_t size, int32_t write)
 */
.globl buffer_validate
.type buffer_validate, function
.section .text, "ax"
buffer_validate:
	add x1, x1, x0
	mrs x3, daif
	msr daifSet, #DAIFSET_IRQ_BIT

loop:
	cbnz w2, 1f
	at S1E0R, x0
	b  2f
1:	at S1E0W, x0
2:	orr x0, x0, #(PAGE_SIZE - 1)
	add x0, x0, #1

	isb
	mrs  x4, PAR_EL1
	tbnz x4, #0, no_permit

	cmp x0, x1
	blo loop

	msr daif, x3
	mov x0, #0
	ret

no_permit:
	msr daif, x3
	mov x0, #-1
	ret
