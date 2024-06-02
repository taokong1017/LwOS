#ifndef __ARM64_TASK_H__
#define __ARM64_TASK_H__

#include <types.h>
#include <fp_context.h>

struct arch_callee_context {
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
	uint64_t fp;
	uint64_t lr;
	uint64_t sp;
};

struct arch_task_context {
	struct arch_callee_context callee_context;
	struct arch_fp_context fp_context;
};

typedef struct arch_task_context arch_task_context_t;
typedef struct arch_callee_context arch_callee_context_t;

#endif