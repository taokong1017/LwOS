#include <gic_v2.h>
#include <isr_table.h>
#include <irq.h>

void arch_irq_enable(uint32_t irq) { arm_gic_irq_enable(irq); }

void arch_irq_disable(uint32_t irq) { arm_gic_irq_disable(irq); }

int arch_irq_is_enabled(uint32_t irq) { return arm_gic_irq_is_enabled(irq); }

int arch_irq_connect(uint32_t irq, uint32_t priority,
					 void (*routine)(const void *), const void *parameter,
					 uint32_t flags) {
	if (irq > ISR_TABLE_SIZE) {
		return -1;
	}

	isr_install(irq, routine, parameter);
	arm_gic_irq_set_priority(irq, priority, flags);

	return 0;
}
