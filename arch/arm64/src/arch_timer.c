#include <arch_timer.h>
#include <irq.h>
#include <gic_v2.h>
#include <log.h>

#define ARCH_TIMER_TAG "ARCH_TIMER"

uint64_t arch_timer_freq_get() { return read_cntfrq_el0(); }

void arch_timer_enable(bool enable) {
	uint64_t cntp_ctl = read_cntp_ctl_el0();

	if (enable) {
		cntp_ctl |= CNTP_CTL_ENABLE;
	} else {
		cntp_ctl &= ~CNTP_CTL_ENABLE;
	}

	write_cntp_ctl_el0(cntp_ctl);
}

void arch_timer_set_irq_mask(bool mask) {
	uint64_t cntp_ctl = read_cntp_ctl_el0();

	if (mask) {
		cntp_ctl |= CNTP_CTL_IMASK;
	} else {
		cntp_ctl &= ~CNTP_CTL_IMASK;
	}

	write_cntp_ctl_el0(cntp_ctl);
}

uint64_t arch_timer_count() { return read_cntpct_el0(); }

void arch_timer_set_compare(uint64_t val) { write_cntp_cval_el0(val); }

uint64_t arch_timer_remaining_count() { return read_cntp_tval_el0(); }

static void arch_timer_compare_isr(const void *arg) { (void)arg; }

void arch_timer_init() {
	bool ret = arch_irq_connect(TICK_IRQ_NUM, GIC_IDLE_PRIO,
								arch_timer_compare_isr, NULL, IRQ_TYPE_EDGE);
	if (!ret) {
		log_err(ARCH_TIMER_TAG, "failed to connect interrupt\n");
	}
}