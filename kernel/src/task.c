#include <types.h>
#include <task.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <arch_task.h>

#define forever() for (;;)
#define ALIGN(start, align) ((start + align - 1) & ~(align - 1))
#define TASK_TO_ID(task) ((task_id_t)task)
#define ID_TO_TASK(task_id) ((struct task *)task_id)

void task_announce() { ; }
extern void arch_main_task_switch(struct task *task);

static errno_t task_params_check(task_id_t *task_id,
								 const char name[TASK_NAME_LEN],
								 task_entry_t entry, void *arg0, void *arg1,
								 void *arg2, void *arg3, uint32_t stack_size) {
	if (!task_id) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!name) {
		return ERRNO_TASK_NAME_EMPTY;
	}

	if (!entry) {
		return ERRNO_TASK_ENTRY_NULL;
	}

	if (!arg0 || !arg1 || !arg2 || !arg3) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (stack_size < TASK_STACK_SIZE_MIN) {
		return ERRNO_TASK_STKSZ_INVALID;
	}

	return OK;
}

static void task_init(struct task *task, const char name[TASK_NAME_LEN],
					  task_entry_t entry, void *arg0, void *arg1, void *arg2,
					  void *arg3, void *stack_ptr, uint32_t stack_size) {
	task->id = TASK_TO_ID(task);
	memcpy(task->name, name, TASK_NAME_LEN);
	task->status = TASK_STATUS_STOP;
	task->stack_ptr = stack_ptr;
	task->stack_size = stack_size;
	task->entry = entry;
	task->args[0] = arg0;
	task->args[1] = arg1;
	task->args[2] = arg2;
	task->args[3] = arg3;
}

errno_t task_create(task_id_t *task_id, const char name[TASK_NAME_LEN],
					task_entry_t entry, void *arg0, void *arg1, void *arg2,
					void *arg3, uint32_t stack_size) {
	uint32_t align_size = 0;
	void *stack_limit = NULL;
	void *stack_ptr = NULL;
	struct task *task = NULL;

	errno_t err = task_params_check(task_id, name, entry, arg0, arg1, arg2,
									arg3, stack_size);
	if (!err) {
		return err;
	}

	align_size = ALIGN(stack_size, TASK_STACK_SIZE_ALIGN);
	stack_limit = mem_alloc_align(align_size, TASK_STACK_ADDR_ALIGN);
	stack_ptr = stack_limit + align_size;
	task = (struct task *)mem_alloc(sizeof(struct task));
	if (!stack_limit || !task) {
		return ERRNO_TASK_NO_MEMORY;
	}

	task_init(task, name, entry, arg0, arg1, arg2, arg3, stack_ptr, align_size);
	arch_task_init(task->id);

	*task_id = task->id;

	return OK;
}

void task_entry_point(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	task->entry(task->args[0], task->args[1], task->args[2], task->args[3]);
}

static void idle_task_entry() { forever(); }

void idle_task_create() {
	task_id_t task_id = 0;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, "idle_task", TASK_NAME_LEN);
	task_create(&task_id, task_name, idle_task_entry, NULL, NULL, NULL, NULL,
				TASK_STACK_SIZE_MIN);
	task_prority_set(task_id, TASK_PRIORITY_LOWEST);
	arch_main_task_switch(ID_TO_TASK(task_id));
}

errno_t task_prority_set(task_id_t task_id, uint32_t prioriy) {
	struct task *task = ID_TO_TASK(task_id);
	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (prioriy < TASK_PRIORITY_LOWEST || prioriy > TASK_PRIORITY_HIGHEST) {
		return ERRNO_TASK_PRIOR_ERROR;
	}

	task->priority = prioriy;
	// TO DO

	return OK;
}
