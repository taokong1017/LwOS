#include <types.h>
#include <cpu_defines.h>
#include <stack_trace.h>
#include <compiler.h>
#include <arch_regs.h>
#include <menuconfig.h>
#include <fp_context.h>
#include <task.h>
#include <task_sched.h>

#define array_size(x) (sizeof(x) / sizeof((x)[0]))

extern uint64_t current_pc_get();

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
	for (int i = 0; i < state->stacks_num; i++) {
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
	state->stack.low = sp + size;

	return true;
}

static bool unwind_next_frame_record(struct unwind_state *state) {
	phys_addr_t fp = state->fp;

	if (fp & 0x7)
		return false;

	if (!unwind_consume_stack(state, fp, 16)) {
		return false;
	}

	/* Jump to last stackframe */
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

	state->fp = regs->gprs[29];
	state->pc = regs->pc;
}

static void unwind_init_from_caller(struct unwind_state *state) {
	unwind_init(state, current_task_get());
	uint64_t fp = 0;

	__asm__ __volatile__("mov %0, x29" : "=r"(fp)::"memory");
	state->fp = (phys_addr_t)fp;
	state->pc = (phys_addr_t)current_pc_get();
}

static void unwind_init_from_task(struct unwind_state *state,
								  struct task *task) {
	unwind_init(state, task);

	state->fp = task_saved_fp(task);
	state->pc = task_saved_lr(task);
}

static bool unwind_next(struct unwind_state *state) {
	return unwind_next_frame_record(state);
}

typedef bool (*unwind_consume_func)(const struct unwind_state *state,
									void *cookie);

static void do_kunwind(struct unwind_state *state,
					   unwind_consume_func consume_state, void *cookie) {
	while (true) {
		if (!consume_state(state, cookie)) {
			break;
		}

		if (!unwind_next(state)) {
			break;
		}
	}
}

static void unwind_stack_walk(unwind_consume_func consume_state, void *cookie,
							  struct task *task, struct arch_regs *regs) {
	struct task *current_task = current_task_get();

	struct stack_info stacks[] = {
		task_stack_info(task == NULL ? current_task : task),
		irq_stack_info(task == NULL ? current_task : task),
	};
	struct unwind_state state = {
		.stacks = stacks,
		.stacks_num = array_size(stacks),
	};

	if (!task && !regs) {
		return;
	}

	if (regs) {
		unwind_init_from_regs(&state, regs);
	} else if (task == current_task) {
		unwind_init_from_caller(&state);
	} else {
		unwind_init_from_task(&state, task);
	}

	do_kunwind(&state, consume_state, cookie);
}

struct unwind_consume_entry_data {
	stack_trace_consume_func consume_entry;
	void *cookie;
};

static bool arch_unwind_consume_entry(const struct unwind_state *state,
									  void *cookie) {
	struct unwind_consume_entry_data *data = cookie;
	return data->consume_entry(data->cookie, state->pc);
}

void arch_stack_walk(stack_trace_consume_func consume_entry, void *cookie,
					 struct task *task, struct arch_regs *regs) {
	struct unwind_consume_entry_data data = {
		.consume_entry = consume_entry,
		.cookie = cookie,
	};

	unwind_stack_walk(arch_unwind_consume_entry, &data, task, regs);
}
