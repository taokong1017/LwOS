#include <menuconfig.h>
#include <arch_syscall.h>
#include <offsets.h>
#include <cpu_defines.h>
#include <cpu.h>
#include <general.h>
#include <pgtable_hwdef.h>

/*
 * void sync_func(struct arch_regs *regs)
 */
.globl sync_func
.type sync_func, function
.section .text, "ax"
sync_func:
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
	mrs x1, esr_el1
	lsr x2, x1, #26
	cmp x2, #0x15 /* 0x15 = svc */
	bne inv

#ifdef CONFIG_USER_SPACE
	and x1, x1, #0xff
	cmp x1, SVC_CALL_SYSTEM_CALL
	b.ne ret
	bl syscall_dispatch
	b ret
#endif

inv:
	bl el0t_64_sync_handler
ret:
	ldp	x29, x30, [sp], #16
	ret

#ifdef CONFIG_USER_SPACE
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
#endif
