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

errno_t user_task_sched_unlock() {
	return arch_syscall_invoke0(SYSCALL_TASK_SCHED_UNLOCK);
}

errno_t user_msgq_create(const char *name, uint32_t max_msgs, uint32_t msg_size,
						 msgq_id_t *id) {
	return arch_syscall_invoke4((uintptr_t)name, max_msgs, msg_size,
								(uintptr_t)id, SYSCALL_MSGQ_CREATE);
}

errno_t user_msgq_send(msgq_id_t id, const void *msg, uint32_t size,
					   uint64_t timeout) {
	return arch_syscall_invoke4((uintptr_t)id, (uintptr_t)msg, size, timeout,
								SYSCALL_MSGQ_SEND);
}

errno_t user_msgq_receive(msgq_id_t id, void *msg, uint32_t *size,
						  uint64_t timeout) {
	return arch_syscall_invoke4((uintptr_t)id, (uintptr_t)msg, (uintptr_t)size,
								timeout, SYSCALL_MSGQ_RECV);
}

errno_t user_msgq_destroy(msgq_id_t id) {
	return arch_syscall_invoke1((uintptr_t)id, SYSCALL_MSGQ_DESTROY);
}

errno_t user_sem_create(const char *name, uint32_t count, uint32_t max_count,
						sem_id_t *id) {
	return arch_syscall_invoke4((uintptr_t)name, count, max_count,
								(uintptr_t)id, SYSCALL_SEM_CREATE);
}

errno_t user_sem_take(sem_id_t id, uint64_t timeout) {
	return arch_syscall_invoke2(id, timeout, SYSCALL_SEM_TAKE);
}

errno_t user_sem_give(sem_id_t id) {
	return arch_syscall_invoke1(id, SYSCALL_SEM_GIVE);
}

errno_t user_sem_destroy(sem_id_t id) {
	return arch_syscall_invoke1(id, SYSCALL_SEM_DESTROY);
}

errno_t user_mutex_create(const char *name, mutex_id_t *id) {
	return arch_syscall_invoke2((uintptr_t)name, (uintptr_t)id,
								SYSCALL_MUTEX_CREATE);
}

errno_t user_mutex_take(mutex_id_t id, uint32_t timeout) {
	return arch_syscall_invoke2(id, timeout, SYSCALL_MUTEX_TAKE);
}

errno_t user_mutex_give(mutex_id_t id) {
	return arch_syscall_invoke1(id, SYSCALL_MUTEX_GIVE);
}

errno_t user_mutex_destroy(mutex_id_t id) {
	return arch_syscall_invoke1(id, SYSCALL_MUTEX_DESTROY);
}
