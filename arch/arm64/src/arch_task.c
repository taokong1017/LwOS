#include <arch_task.h>
#include <task.h>

extern void task_entry_point(task_id_t task_id);

void arch_task_init(task_id_t task_id) {
	struct arch_esf_context *esf = NULL;
	struct task *task = ID_TO_TASK(task_id);

	task->stack_ptr -= sizeof(struct arch_esf_context);
	esf = (struct arch_esf_context *)task->stack_ptr;
	esf->x0 = (uint64_t)task_id;
	esf->x1 = (uint64_t)task_entry_point;
}
