#include <smp.h>
#include <arch_atomic.h>
#include <menuconfig.h>
#include <tick.h>
#include <percpu.h>
#include <arch_smp.h>
#include <compiler.h>
#include <task_sched.h>

typedef void (*smp_init_func)(void *arg);
struct smp_init_callback {
	smp_init_func init_func;
	void *arg;
};

static atomic_t cpu_start_flag = 0;
static atomic_t ready_flag = 0;

void smp_init() {
	uint32_t i = 0;
	atomic_clear(&cpu_start_flag);

	for (i = 1; i < CONFIG_CPUS_MAX_NUM; i++) {
		smp_cpu_start(i);
	}

	atomic_set(&cpu_start_flag, (atomic_t)1);
}

static void smp_cpu_start_callback(void *arg) {
	struct smp_init_callback *cb = arg;

	atomic_set(&ready_flag, 1);
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
	atomic_clear(&ready_flag);

	arch_cpu_start(cpu_id, smp_cpu_start_callback, NULL);

	while (!atomic_get(&ready_flag)) {
		udelay(100);
	}
}
