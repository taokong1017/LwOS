#ifndef __SMP_H__
#define __SMP_H__

#include <types.h>
#include <menuconfig.h>

#define ALL_CPU_MASK (((1U) << CONFIG_CPUS_MAX_NUM) - 1)

enum smp_ipi_type {
	SMP_IPI_SCHED = 0,
	SMP_IPI_HALT,
};

void smp_init();
void smp_cpu_start(uint32_t cpu_id);
void smp_sched_notify();
void smp_halt_notify();
void smp_sched_handler(const void *arg);
void smp_halt_handler(const void *arg);

#endif
