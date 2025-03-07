#ifndef __ARM64_ISR_H__
#define __ARM64_ISR_H__

#define UART_IRQ_NUM 33
#define TICK_IRQ_NUM 27
#define IRQ_MAX_NUM 1020

#ifndef __ASSEMBLY__
#include <arch_regs.h>

typedef void (*irq_routine_t)(const void *parameter);

void arch_irq_enable(uint32_t irq);
void arch_irq_disable(uint32_t irq);
bool arch_irq_is_enabled(uint32_t irq);
bool arch_irq_connect(uint32_t irq, uint32_t priority,
					  void (*routine)(const void *), const void *parameter,
					  uint32_t flags);

uint32_t arch_irq_save();
uint32_t arch_irq_status();
void arch_irq_restore(uint32_t key);
void arch_irq_unlock();
void arch_irq_unlock_with_regs(struct arch_regs *regs);
#endif

#endif
