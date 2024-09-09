#ifndef __ARM64_STACK_TRACE_H__
#define __ARM64_STACK_TRACE_H__

#include <task.h>
#include <arch_regs.h>

struct stack_info {
	virt_addr_t low;
	virt_addr_t high;
};

struct unwind_state {
	virt_addr_t fp;
	virt_addr_t pc;

	struct stack_info stack;
	struct stack_info *stacks;
	uint32_t stacks_num;
};

typedef bool (*stack_trace_consume_func)(void *cookie, virt_addr_t pc,
										 virt_addr_t fp);

void arch_stack_walk(stack_trace_consume_func consume_entry, void *cookie,
					 struct task *task, struct arch_regs *regs);

void arch_stack_default_walk(char *tag, struct task *task,
							 struct arch_regs *regs);

#endif
