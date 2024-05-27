#ifndef __ARM64_ISR_H__
#define __ARM64_ISR_H__

void arch_irq_enable(uint32_t irq);

void arch_irq_disable(uint32_t irq);

int arch_irq_is_enabled(uint32_t irq);

int arch_irq_connect(uint32_t irq, uint32_t priority,
					 void (*routine)(const void *), const void *parameter,
					 uint32_t flags);

#endif
