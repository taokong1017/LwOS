#include <kernel.h>
#include <cpu.h>

#define current_percpu kernel.percpus[arch_cpu_id_get()]

static struct kernel kernel;

struct per_cpu *current_percpu_get() {
	return &current_percpu;
}

struct per_cpu *percpu_get(int cpu_id) {
	return &kernel.percpus[cpu_id];
}
