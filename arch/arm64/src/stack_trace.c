#include <types.h>
#include <cpu_defines.h>
#include <stack_trace.h>
#include <compiler.h>
#include <arch_regs.h>
#include <menuconfig.h>
#include <fp_context.h>
#include <task.h>
#include <task_sched.h>

static struct stack_info stackinfo_get_unknown() {
	struct stack_info stack = {
		.low = 0,
		.high = 0,
	};

	return stack;
}

static bool stackinfo_on_stack(const struct stack_info *info, phys_addr_t sp,
							   phys_addr_t size) {
	if (!info->low) {
		return false;
	}

	if (sp < info->low || sp + size < sp || sp + size > info->high) {
		return false;
	}

	return true;
}

static struct stack_info *
unwind_find_next_stack(const struct unwind_state *state, phys_addr_t sp,
					   size_t size) {
	for (int i = 0; i < state->nr_stacks; i++) {
		struct stack_info *info = &state->stacks[i];

		if (stackinfo_on_stack(info, sp, size)) {
			return info;
		}
	}

	return NULL;
}

static bool unwind_consume_stack(struct unwind_state *state, phys_addr_t sp,
								 size_t size) {
	struct stack_info *next;

	if (stackinfo_on_stack(&state->stack, sp, size)) {
		state->stack.low = sp + size;
		return true;
	}

	next = unwind_find_next_stack(state, sp, size);
	if (!next) {
		return false;
	}

	state->stack = *next;
	*next = stackinfo_get_unknown();
	state->stack.low = sp + size;

	return true;
}

static bool unwind_next_frame_record(struct unwind_state *state) {
	phys_addr_t fp = state->fp;

	if (fp & 0x7)
		return false;

	if (unwind_consume_stack(state, fp, 16)) {
		return false;
	}

	state->fp = read_once(*(phys_addr_t *)(fp));
	state->pc = read_once(*(phys_addr_t *)(fp + 8));

	return true;
}

static void unwind_init(struct unwind_state *state, struct task *task) {
	state->stack = stackinfo_get_unknown();
	state->task = task;
}

static void unwind_init_from_regs(struct unwind_state *state,
								  struct arch_regs *regs) {
	unwind_init(state, current_task_get());

	state->fp = regs->gprs[ARM_ARCH_REGS_GPR29];
	state->pc = regs->pc;
}

static void unwind_init_from_caller(struct unwind_state *state) {
	unwind_init(state, current_task_get());

	state->fp = (phys_addr_t)__builtin_frame_address(1);
	state->pc = (phys_addr_t)__builtin_return_address(0);
}

static void unwind_init_from_task(struct unwind_state *state,
								  struct task *task) {
	unwind_init(state, task);

	state->fp = task->task_context.callee_context.x29;
	state->pc = task->task_context.callee_context.x30;
}
