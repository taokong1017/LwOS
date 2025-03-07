#ifndef __SMP_H__
#define __SMP_H__

#include <types.h>
#include <menuconfig.h>

enum smp_ipi_type {
	SMP_IPI_SCHED = 0,
	SMP_IPI_NUM,
	USER_IPI_MIN = SMP_IPI_NUM,
};

void smp_init();
void smp_cpu_start(uint32_t cpu_id);
void smp_sched_notify();
void smp_sched_handler(const void *arg);

#endif
