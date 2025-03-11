#include <smp.h>
#include <arch_atomic.h>
#include <menuconfig.h>
#include <tick.h>
#include <percpu.h>
#include <arch_smp.h>
#include <compiler.h>
#include <task_sched.h>
#include <gic_v2.h>
#include <irq.h>
#include <cpu.h>
#include <log.h>

#define LOG_TAG "SMP"
#define ALL_CPU_MASK (((1U) << CONFIG_CPUS_MAX_NUM) - 1)

typedef void (*smp_init_func)(void *arg);
struct smp_init_callback {
	smp_init_func init_func;
	void *arg;
};

static atomic_t cpu_start_flag = 0;

void smp_init() {
	uint32_t i = 0;

	arch_irq_connect(SMP_IPI_SCHED, 160, smp_sched_handler, NULL,
					 IRQ_TYPE_LEVEL);

	atomic_clear(&cpu_start_flag);
	for (i = 1; i < CONFIG_CPUS_MAX_NUM; i++) {
		smp_cpu_start(i);
	}

	atomic_set(&cpu_start_flag, (atomic_t)1);
}

static void smp_cpu_start_callback(void *arg) {
	struct smp_init_callback *cb = arg;

	while (!atomic_get(&cpu_start_flag)) {
		udelay(100);
	}

	if (cb && cb->init_func) {
		cb->init_func(cb->arg);
	}

	task_sched_start();

	return;
}

void smp_cpu_start(uint32_t cpu_id) {
	arch_cpu_start(cpu_id, smp_cpu_start_callback, NULL);
}

void smp_sched_notify() {
	uint32_t cur_cpu_id = arch_cpu_id_get();
	uint32_t mask = ALL_CPU_MASK ^ (1U << cur_cpu_id);
	uint64_t no_use_affi = 0;

	gic_raise_sgi(SMP_IPI_SCHED, no_use_affi, mask);
}

void smp_sched_handler(const void *arg) {
	(void)arg;
	struct per_cpu *percpu = current_percpu_get();

	percpu->pend_sched = true;
	return;
}
