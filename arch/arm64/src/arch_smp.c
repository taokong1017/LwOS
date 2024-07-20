#include <arch_smp.h>
#include <cpu_ops.h>
#include <compiler.h>
#include <cache.h>
#include <percpu.h>
#include <gic_v2.h>

static struct boot_params arch_boot_params = {
	.arg = NULL, .cpu_id = 0, .func = NULL};
const static uint32_t cpu_mp_ids[] = {0x0,	 0x1,	0x2,   0x3,
									  0x100, 0x101, 0x102, 0x103};
extern void __start();

void arch_cpu_start(uint32_t cpu_id, arch_cpu_start_func func, void *arg) {
	arch_boot_params.arg = arg;
	arch_boot_params.cpu_id = cpu_id;
	arch_boot_params.func = func;
	arch_boot_params.mp_id = cpu_mp_ids[cpu_id];
	dsb(osh);

	dcache_clean_inval_poc((uint64_t)&arch_boot_params,
						   (uint64_t)&arch_boot_params +
							   sizeof(struct boot_params));
	cpu_on(arch_boot_params.mp_id, (uintptr_t)__start);

	while (arch_boot_params.func) {
		wfe();
	}
}

void arch_secondary_cpu_init() {
	arch_cpu_start_func func = NULL;
	void *arg = NULL;

	percpu_init(arch_boot_params.cpu_id);
	arm_gic_init(false);

	func = arch_boot_params.func;
	arg = arch_boot_params.arg;
	dsb(osh);

	arch_boot_params.func = NULL;
	dsb(osh);
	sev();

	if (func) {
		func(arg);
	}
}
