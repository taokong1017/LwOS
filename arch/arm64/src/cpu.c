#include <types.h>
#include <cpu.h>
#include <operate_regs.h>
#include <menuconfig.h>

uint64_t cpu_map[CONFIG_CPUS_MAX_NUM] = {[0 ... CONFIG_CPUS_MAX_NUM - 1] =
											 (uint64_t)(-1)};

uint32_t arch_cpu_id_get() {
	uint32_t cpu_id = 0;

#ifdef CONFIG_SMP
	uint64_t mpidr = read_mpidr_el1();

	if (mpidr & MPIDR_MT_MASK) {
		cpu_id = MPIDR_AFFLVL(mpidr, 1);
	} else {
		cpu_id = MPIDR_AFFLVL(mpidr, 0);
	}
#endif

	return cpu_id;
}
