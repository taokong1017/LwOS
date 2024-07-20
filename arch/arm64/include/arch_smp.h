#ifndef __ARM64_SMP_H__
#define __ARM64_SMP_H__

#include <types.h>

typedef void (*arch_cpu_start_func)(void *arg);
struct boot_params {
	uint32_t cpu_id;
	uint64_t mp_id;
	arch_cpu_start_func func;
	void *arg;
};

void arch_cpu_start(uint32_t cpu_id, arch_cpu_start_func func, void *arg);
uint64_t arch_cpu_num_get();
uint64_t arch_cpu_mpid_get(uint32_t cpu_id);
uint64_t arch_cpu_master_mpid_get();

#endif
