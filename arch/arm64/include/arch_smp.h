#ifndef __ARM64_SMP_H__
#define __ARM64_SMP_H__

#include <types.h>

typedef void (*arch_cpu_start_func)(void *arg);
struct boot_params {
	uint32_t cpu_id;
	uint32_t mp_id;
	arch_cpu_start_func func;
	void *arg;
};

void arch_cpu_start(uint32_t cpu_id, arch_cpu_start_func func, void *arg);

#endif
