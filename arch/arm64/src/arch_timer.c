#include <arch_timer.h>
#include <irq.h>
#include <gic_v2.h>
#include <log.h>
#include <tick.h>
#include <menuconfig.h>
#include <compiler.h>
#include <mem_domain.h>

#define ARCH_TIMER_TAG "ARCH_TIMER"
#define CYC_PER_TICK (arch_timer_freq_get() / CONFIG_TICKS_PER_SECOND)

uint64_t arch_timer_freq_get() { return read_cntfrq_el0(); }

void arch_timer_freq_set(uint64_t freq) { write_cntfrq_el0(freq); }

void arch_timer_enable(bool enable) {
	uint64_t cntv_ctl = read_cntv_ctl_el0();

	if (enable) {
		cntv_ctl |= CNTV_CTL_ENABLE;
	} else {
		cntv_ctl &= ~CNTV_CTL_ENABLE;
	}

	write_cntv_ctl_el0(cntv_ctl);
}

void arch_timer_set_irq_mask(bool mask) {
	uint64_t cntv_ctl = read_cntv_ctl_el0();

	if (mask) {
		cntv_ctl |= CNTV_CTL_IMASK;
	} else {
		cntv_ctl &= ~CNTV_CTL_IMASK;
	}

	write_cntv_ctl_el0(cntv_ctl);
}

uint64_t arch_timer_count() {
	dsb(sy);
	isb();
	return read_cntvct_el0();
}

void arch_timer_set_compare(uint64_t val) { write_cntv_cval_el0(val); }

void arch_timer_compare_isr(const void *arg) {
	(void)arg;

	ttbr_t ttbr = mem_domain_save();
	tick_announce();
	arch_timer_set_compare(arch_timer_count() + CYC_PER_TICK);
	arch_timer_enable(true);
	arch_timer_set_irq_mask(false);
	mem_domain_restore(ttbr);

	return;
}

void arch_timer_init(bool primary) {
	if (primary) {
		bool ret = arch_irq_connect(TICK_IRQ_NUM, 160, arch_timer_compare_isr,
									NULL, IRQ_TYPE_LEVEL);
		if (!ret) {
			log_err(ARCH_TIMER_TAG, "failed to connect interrupt\n");
			return;
		}
	}

	arch_irq_enable(TICK_IRQ_NUM);
	arch_timer_set_irq_mask(false);

	arch_timer_enable(false);
	arch_timer_set_compare(arch_timer_count() + CYC_PER_TICK);
	arch_timer_enable(true);
}
