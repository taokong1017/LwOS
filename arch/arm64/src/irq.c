#include <gic_v2.h>
#include <isr_table.h>
#include <irq.h>
#include <cpu.h>

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
	uint32_t key;
	__asm__ __volatile__("mrs %0, daif \n"
						 "msr daifset, #0xf"
						 : "=r"(key)
						 :
						 : "memory");
	return key;
}

void arch_irq_restore(uint32_t key) {
	__asm__ __volatile__("msr daif, %0 " ::"r"(key) : "memory");
}

uint32_t arch_irq_unlock() {
	uint32_t key;
	__asm__ __volatile__("mrs %0, daif \n"
						 "msr daifclr, #0xf"
						 : "=r"(key)
						 :
						 : "memory");
	return key;
}

bool arch_irq_lock_check(uint32_t key) { return (key & DAIF_IRQ_BIT) != 0; }
