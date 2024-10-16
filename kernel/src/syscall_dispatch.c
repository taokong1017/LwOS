#include <syscall.h>
#include <log.h>
#include <compiler.h>
#include <task.h>
#include <percpu.h>
#include <task_sched.h>

#define SYSCALL_TAG "SYSCALL"
typedef uintptr_t (*syscall_handler_t)(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs);

static uintptr_t syscall_log_level_set(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	int32_t level = (int32_t)arg1;

	return log_level_set(level);
}

static uintptr_t syscall_log_level_get(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	return log_level_get();
}

static uintptr_t syscall_task_create(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 void *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t *task_id = (task_id_t *)arg1;
	struct task_info *task_info = (struct task_info *)arg2;

	return task_create_with_stack(
		task_id, task_info->name, task_info->entry, task_info->arg0,
		task_info->arg1, task_info->arg2, task_info->arg3, task_info->task,
		task_info->stack, task_info->stack_size, task_info->flag);
}

static uintptr_t syscall_task_start(uintptr_t arg1, uintptr_t arg2,
									uintptr_t arg3, uintptr_t arg4,
									uintptr_t arg5, uintptr_t arg6,
									void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;

	return task_start(task_id);
}

static uintptr_t syscall_task_stop(uintptr_t arg1, uintptr_t arg2,
								   uintptr_t arg3, uintptr_t arg4,
								   uintptr_t arg5, uintptr_t arg6, void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;

	return task_stop(task_id);
}

static uintptr_t syscall_task_delay(uintptr_t arg1, uintptr_t arg2,
									uintptr_t arg3, uintptr_t arg4,
									uintptr_t arg5, uintptr_t arg6,
									void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	uint64_t ticks = (uint64_t)arg1;

	return task_delay(ticks);
}

static uintptr_t syscall_task_suspend(uintptr_t arg1, uintptr_t arg2,
									  uintptr_t arg3, uintptr_t arg4,
									  uintptr_t arg5, uintptr_t arg6,
									  void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;

	return task_stop(task_id);
}

static uintptr_t syscall_task_resume(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;

	return task_resume(task_id);
}

static uintptr_t syscall_task_prio_set(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;
	uint32_t prioriy = (uint32_t)arg2;

	return task_priority_set(task_id, prioriy);
}

static uintptr_t syscall_task_prio_get(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;
	uint32_t *prioriy = (uint32_t *)arg2;

	return task_priority_get(task_id, prioriy);
}

static uintptr_t syscall_task_self_id(uintptr_t arg1, uintptr_t arg2,
									  uintptr_t arg3, uintptr_t arg4,
									  uintptr_t arg5, uintptr_t arg6,
									  void *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t *task_id = (task_id_t *)arg1;

	*task_id = task_self_id();

	return OK;
}

static uintptr_t default_syscall_handler(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 void *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	log_info(SYSCALL_TAG, "enter default syscall handler\n");
	forever();
	code_unreachable();
}

const syscall_handler_t syscall_table[SYSCALL_ID_LIMIT + 1] = {
	[SYSCALL_LOG_LEVEL_GET] = syscall_log_level_get,
	[SYSCALL_LOG_LEVEL_SET] = syscall_log_level_set,
	[SYSCALL_TASK_CREATE] = syscall_task_create,
	[SYSCALL_TASK_START] = syscall_task_start,
	[SYSCALL_TASK_STOP] = syscall_task_stop,
	[SYSCALL_TASK_DELAY] = syscall_task_delay,
	[SYSCALL_TASK_SUSPEND] = syscall_task_suspend,
	[SYSCALL_TASK_RESUME] = syscall_task_resume,
	[SYSCALL_TASK_PRIO_SET] = syscall_task_prio_set,
	[SYSCALL_TASK_PRIO_GET] = syscall_task_prio_get,
	[SYSCALL_TASK_SELF_ID] = syscall_task_self_id,
	[SYSCALL_ID_LIMIT] = default_syscall_handler,
};

uint32_t syscall_table_size_get() { return ARRAY_SIZE(syscall_table); }
