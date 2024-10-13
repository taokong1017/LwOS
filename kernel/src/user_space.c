#include <user_space.h>
#include <arch_syscall.h>
#include <syscall.h>

bool user_log_level_set(int32_t level) {
	return arch_syscall_invoke1(level, SYSCALL_LOG_LEVEL_SET);
}

int32_t user_log_level_get() {
	return arch_syscall_invoke0(SYSCALL_LOG_LEVEL_GET);
}

errno_t user_task_create(task_id_t *task_id, const char *name,
						 task_entry_func entry, void *arg0, void *arg1,
						 void *arg2, void *arg3, struct task *task, void *stack,
						 uint32_t stack_size, uint32_t flag) {

	struct task_info task_info = {
		.name = name,
		.task = task,
		.entry = entry,
		.arg0 = arg0,
		.arg1 = arg1,
		.arg2 = arg2,
		.arg3 = arg3,
		.stack = stack,
		.stack_size = stack_size,
		.flag = flag,
	};

	return arch_syscall_invoke2((uintptr_t)task_id, (uintptr_t)&task_info,
								SYSCALL_TASK_CREATE);
}

errno_t user_task_start(task_id_t task_id) {
	return arch_syscall_invoke1((uintptr_t)task_id, SYSCALL_TASK_START);
}

errno_t user_task_stop(task_id_t task_id) {
	return arch_syscall_invoke1((uintptr_t)task_id, SYSCALL_TASK_STOP);
}

errno_t user_task_delay(uint64_t ticks) {
	return arch_syscall_invoke1(ticks, SYSCALL_TASK_DELAY);
}

errno_t user_task_suspend(task_id_t task_id) {
	return arch_syscall_invoke1(task_id, SYSCALL_TASK_SUSPEND);
}

errno_t user_task_resume(task_id_t task_id) {
	return arch_syscall_invoke1(task_id, SYSCALL_TASK_RESUME);
}

errno_t user_task_priority_set(task_id_t task_id, uint32_t prioriy) {
	return arch_syscall_invoke2(task_id, prioriy, SYSCALL_TASK_PRIO_SET);
}

errno_t user_task_priority_get(task_id_t task_id, uint32_t *prioriy) {
	return arch_syscall_invoke2(task_id, (uintptr_t)prioriy,
								SYSCALL_TASK_PRIO_GET);
}

errno_t user_task_self_id(task_id_t *task_id) {
	return arch_syscall_invoke1((uintptr_t)task_id, SYSCALL_TASK_SELF_ID);
}
