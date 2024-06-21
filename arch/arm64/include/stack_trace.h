#ifndef __ARM64_STACK_TRACE_H__
#define __ARM64_STACK_TRACE_H__

struct stack_info {
	phys_addr_t low;
	phys_addr_t high;
};

struct unwind_state {
	struct task *task;
	phys_addr_t fp;
	phys_addr_t pc;

	struct stack_info stack;
	struct stack_info *stacks;
	uint32_t nr_stacks;
};

#endif
