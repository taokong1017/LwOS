#include <gic_v2.h>
#include <isr_table.h>
#include <irq.h>
#include <cpu.h>
#include <task_sched.h>

void arch_irq_enable(uint32_t irq) { arm_gic_irq_enable(irq); }

void arch_irq_disable(uint32_t irq) { arm_gic_irq_disable(irq); }

bool arch_irq_is_enabled(uint32_t irq) { return arm_gic_irq_is_enabled(irq); }

bool arch_irq_connect(uint32_t irq, uint32_t priority,
					  void (*routine)(const void *), const void *parameter,
					  uint32_t flags) {
	if (irq > IRQ_MAX_NUM) {
		return false;
	}

	isr_install(irq, routine, parameter);
	arm_gic_irq_set_priority(irq, priority, flags);

	return true;
}

uint32_t arch_irq_save() {
	uint32_t key = 0;

	__asm__ __volatile__("mrs %0, daif \n"
						 "msr daifSet, %1"
						 : "=r"(key)
						 : "i"(DAIFSET_IRQ_BIT | DAIFSET_FIQ_BIT)
						 : "memory");
	return key;
}

void arch_irq_restore(uint32_t key) {
	__asm__ __volatile__("msr daif, %0" ::"r"(key) : "memory");
}

void arch_irq_unlock() {
	__asm__ __volatile__(
		"msr daifclr, %0" ::"i"(DAIFCLR_FIQ_BIT | DAIFCLR_IRQ_BIT)
		: "memory");
}

uint32_t arch_irq_status() {
	uint32_t key = 0;

	__asm__ __volatile__("mrs %0, daif" : "=r"(key) : : "memory");
	return key;
}

void arch_irq_unlock_with_regs(struct arch_regs *regs) {
	if (!regs) {
		return;
	}

	regs->pstate &= ~(DAIF_FIQ_BIT | DAIF_IRQ_BIT);
}
