#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <types.h>
#include <menuconfig.h>
#include <percpu.h>

#define WAIT_FOREVER -1
#define NO_WAIT 0

struct kernel {
	struct per_cpu percpus[CONFIG_CPUS_MAX_NUM];
};

struct per_cpu *current_percpu_get();
struct per_cpu *percpu_get(int cpu_id);

#endif
