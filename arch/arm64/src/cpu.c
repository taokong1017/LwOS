#include <types.h>
#include <cpu.h>
#include <operate_regs.h>
#include <menuconfig.h>
#include <compiler.h>
#include <arch_smp.h>
#include <log.h>

#define CPU_TAG "CPU"

uint32_t arch_cpu_id_get() {
	uint32_t i = 0;
	uint32_t cpu_id = -1;
	uint64_t mpidr = 0;

	mpidr = read_mpidr_el1() & 0xffffff;
	for (i = 0; i < arch_cpu_num_get(); i++) {
		if (arch_cpu_mpid_get(i) == mpidr) {
			cpu_id = i;
			break;
		}
	}

	if (cpu_id == -1) {
		log_fatal(CPU_TAG, "invalid cpu id: %d\n", cpu_id);
	}

	return cpu_id;
}
