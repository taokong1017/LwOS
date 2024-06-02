#include <types.h>
#include <task.h>
#include <errno.h>
#include <memory.h>
#include <string.h>

#define ALIGN(start, align) ((start + align - 1) & ~(align - 1))
#define TASK_TO_ID(task) ((task_id_t)task)

void task_announce() { ; }

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

	if (!stack_size) {
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
	if (!stack_limit || stack_ptr || task) {
		return ERRNO_TASK_NO_MEMORY;
	}

	task_init(task, name, entry, arg0, arg1, arg2, arg3, stack_ptr, align_size);
	*task_id = task->id;

	return OK;
}
