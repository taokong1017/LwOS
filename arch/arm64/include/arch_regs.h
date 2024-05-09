#ifndef __ARM64__REGS_H__
#define __ARM64__REGS_H__

#include <types.h>
#include <cpu_defines.h>

struct arch_regs {
	/* X0 - X29 */
	uint64_t gpr[CPU_GPR_COUNT];
	/* Link Register (or X30) */
	uint64_t lr;
	/* Stack Pointer */
	uint64_t sp;
	/* Program Counter */
	uint64_t pc;
	/* PState/CPSR */
	uint64_t pstate;
} __packed;

#endif
