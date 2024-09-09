#include <types.h>
#include <cpu_defines.h>
#include <compiler.h>
#include <arch_regs.h>
#include <menuconfig.h>
#include <fp_context.h>
#include <task.h>
#include <task_sched.h>
#include <stack_trace.h>
#include <stdio.h>
#include <percpu.h>
#include <cpu.h>

#define array_size(x) (sizeof(x) / sizeof((x)[0]))
#define IS_VALID_ADDR(addr) (addr && addr != -1)

extern uint64_t current_pc_get();

static struct stack_info stackinfo_get_unknown() {
	struct stack_info stack = {
		.low = 0,
		.high = 0,
	};

	return stack;
}

static bool stackinfo_on_stack(const struct stack_info *info, virt_addr_t sp,
							   size_t size) {
	if (!info->low) {
		return false;
	}

	if (sp < info->low || sp + size < sp || sp + size > info->high) {
		return false;
	}

	return true;
}

static struct stack_info *
unwind_find_next_stack(const struct unwind_state *state, virt_addr_t sp,
					   size_t size) {
	for (int32_t i = 0; i < state->stacks_num; i++) {
		struct stack_info *info = &state->stacks[i];

		if (stackinfo_on_stack(info, sp, size)) {
			return info;
		}
	}

	return NULL;
}

static bool unwind_consume_stack(struct unwind_state *state, virt_addr_t sp,
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
	virt_addr_t fp = state->fp;

	if (fp & 0x7) {
		return false;
	}

	if (!unwind_consume_stack(state, fp, 16)) {
		return false;
	}

	/* Jump to last stackframe */
	state->fp = read_once(*(virt_addr_t *)(fp));
	state->pc = read_once(*(virt_addr_t *)(fp + 8));

	return true;
}

static void unwind_init_from_regs(struct unwind_state *state,
								  struct arch_regs *regs) {
	state->fp = regs->gprs[29];
	state->pc = regs->pc;
	state->stack = stackinfo_get_unknown();
}

static void unwind_init_from_caller(struct unwind_state *state) {
	uint64_t fp = 0;

	__asm__ __volatile__("mov %0, x29" : "=r"(fp)::"memory");
	state->fp = (virt_addr_t)fp;
	state->pc = (virt_addr_t)current_pc_get();
	state->stack = stackinfo_get_unknown();
}

static void unwind_init_from_task(struct unwind_state *state,
								  struct task *task) {
	state->fp = task_saved_fp(task);
	state->pc = task_saved_lr(task);
	state->stack = stackinfo_get_unknown();
}

static void unwind_init_from_irq(struct unwind_state *state) {
	uint64_t fp = 0;

	__asm__ __volatile__("mov %0, x29" : "=r"(fp)::"memory");
	state->fp = (virt_addr_t)fp;
	state->pc = (virt_addr_t)current_pc_get();
	state->stack = stackinfo_get_unknown();
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
		irq_stack_info(arch_cpu_id_get()),
	};
	struct unwind_state state = {
		.stacks = stacks,
		.stacks_num = array_size(stacks),
	};

	if (regs) {
		unwind_init_from_regs(&state, regs);
	} else if (task == current_task) {
		unwind_init_from_caller(&state);
	} else if (task) {
		unwind_init_from_task(&state, task);
	} else {
		unwind_init_from_irq(&state);
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
	return data->consume_entry(data->cookie, state->pc, state->fp);
}

void arch_stack_walk(stack_trace_consume_func consume_entry, void *cookie,
					 struct task *task, struct arch_regs *regs) {
	struct unwind_consume_entry_data data = {
		.consume_entry = consume_entry,
		.cookie = cookie,
	};

	unwind_stack_walk(arch_unwind_consume_entry, &data, task, regs);
}

static bool show_Linker(void *cookie, virt_addr_t pc, virt_addr_t fp) {
	uint32_t *level = (uint32_t *)cookie;

	if (IS_VALID_ADDR(pc)) {
		printf("  %u: 0x%016llx - 0x%016llx\n", (*level)++, fp, pc);
		return true;
	} else {
		return false;
	}
}

void arch_stack_default_walk(char *tag, struct task *task,
							 struct arch_regs *regs) {
	uint32_t level = 0;

	if (tag == NULL) {
		printf("stack trace:\n");
	} else {
		printf("[%s] stack trace:\n", tag);
	}

	arch_stack_walk(show_Linker, &level, task, regs);

	return;
}
