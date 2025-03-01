#ifndef __ARM64_TASK_H__
#define __ARM64_TASK_H__

#include <types.h>
#include <general.h>
#include <fp_context.h>
#include <compiler.h>
#include <menuconfig.h>

typedef long task_id_t;

struct arch_esf_context {
	uint64_t x0;
	uint64_t x1;
	uint64_t x2;
	uint64_t x3;
	uint64_t x4;
	uint64_t x5;
	uint64_t x6;
	uint64_t x7;
	uint64_t x8;
	uint64_t x9;
	uint64_t x10;
	uint64_t x11;
	uint64_t x12;
	uint64_t x13;
	uint64_t x14;
	uint64_t x15;
	uint64_t x16;
	uint64_t x17;
	uint64_t x18;
	uint64_t lr;
	uint64_t spsr;
	uint64_t elr;
} ALIGNED(16);

struct arch_callee_context {
	uint64_t x0;
	uint64_t x1;
	uint64_t x2;
	uint64_t x3;
	uint64_t x4;
	uint64_t x5;
	uint64_t x6;
	uint64_t x7;
	uint64_t x8;
	uint64_t x9;
	uint64_t x10;
	uint64_t x11;
	uint64_t x12;
	uint64_t x13;
	uint64_t x14;
	uint64_t x15;
	uint64_t x16;
	uint64_t x17;
	uint64_t x18;
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t x29; /* frame pointer */
	uint64_t x30; /* link register */
	uint64_t sp;
	uint64_t daif;
	uint64_t nzcv;
} ALIGNED(16);

struct arch_task_context {
	struct arch_callee_context callee_context;
#ifdef CONFIG_FPU_ENABLE
	struct arch_fp_context fp_context;
#endif
};

typedef struct arch_task_context arch_task_context_t;
typedef struct arch_callee_context arch_callee_context_t;

void arch_task_init(task_id_t task_id);
void arch_task_context_switch(arch_task_context_t *new,
							  arch_task_context_t *old);
void arch_main_task_switch(task_id_t task_id);

#endif