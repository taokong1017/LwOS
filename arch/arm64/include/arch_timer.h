#ifndef __ARM64_ARCH_TIMER_H__
#define __ARM64_ARCH_TIMER_H__

#include <types.h>
#include <operate_regs.h>

#define CNTV_CTL_ENABLE 1UL
#define CNTV_CTL_IMASK 2UL

uint64_t arch_timer_freq_get();
void arch_timer_freq_set(uint64_t freq);
void arch_timer_enable(bool enable);
void arch_timer_set_irq_mask(bool mask);
uint64_t arch_timer_count();
void arch_timer_set_compare(uint64_t val);
void arch_timer_init(bool primary);

#endif
