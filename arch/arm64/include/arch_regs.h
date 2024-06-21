#ifndef __ARM64_REGS_H__
#define __ARM64_REGS_H__

#include <types.h>
#include <cpu_defines.h>
#include <compiler.h>

struct arch_regs {
	/* X0 - X29 */
	uint64_t gprs[CPU_GPR_COUNT];
	/* Link Register (or X30) */
	uint64_t lr;
	/* Stack Pointer */
	uint64_t sp;
	/* Program Counter */
	uint64_t pc;
	/* PState/CPSR */
	uint64_t pstate;
} ALIGNED(16);

#endif
