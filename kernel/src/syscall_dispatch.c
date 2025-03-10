#include <arch_regs.h>
#include <syscall.h>
#include <log.h>
#include <compiler.h>
#include <task.h>
#include <percpu.h>
#include <task_sched.h>
#include <sync_exc.h>
#include <irq.h>
#include <msgq.h>
#include <sem.h>
#include <mutex.h>
#include <mem_domain.h>

#define SYSCALL_TAG "SYSCALL"
typedef uintptr_t (*syscall_handler_t)(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   struct arch_regs *regs);

static uintptr_t syscall_log_level_set(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   struct arch_regs *regs) {
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
									   struct arch_regs *regs) {
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
									 struct arch_regs *regs) {
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
									struct arch_regs *regs) {
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
								   uintptr_t arg5, uintptr_t arg6,
								   struct arch_regs *regs) {
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
									struct arch_regs *regs) {
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
									  struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t task_id = (task_id_t)arg1;

	return task_suspend(task_id);
}

static uintptr_t syscall_task_resume(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 struct arch_regs *regs) {
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
									   struct arch_regs *regs) {
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
									   struct arch_regs *regs) {
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
									  struct arch_regs *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	task_id_t *task_id = (task_id_t *)arg1;

	*task_id = task_self_id();

	return OK;
}

static uintptr_t syscall_task_sched_unlock(uintptr_t arg1, uintptr_t arg2,
										   uintptr_t arg3, uintptr_t arg4,
										   uintptr_t arg5, uintptr_t arg6,
										   struct arch_regs *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;

	uint32_t key = arch_irq_save();
	arch_irq_unlock_with_regs(regs);
	task_sched_unlock();
	arch_irq_restore(key);

	return OK;
}

static uintptr_t syscall_msgq_create(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 struct arch_regs *regs) {
	(void)arg5;
	(void)arg6;
	(void)regs;
	const char *name = (const char *)arg1;
	uint32_t max_msgs = (uint32_t)arg2;
	uint32_t msg_size = (uint32_t)arg3;
	msgq_id_t *id = (msgq_id_t *)arg4;

	return msgq_create(name, max_msgs, msg_size, id);
}

static uintptr_t syscall_msgq_send(uintptr_t arg1, uintptr_t arg2,
								   uintptr_t arg3, uintptr_t arg4,
								   uintptr_t arg5, uintptr_t arg6,
								   struct arch_regs *regs) {
	(void)arg5;
	(void)arg6;
	(void)regs;
	msgq_id_t id = (msgq_id_t)arg1;
	const void *msg = (const void *)arg2;
	uint32_t size = (uint32_t)arg3;
	uint64_t timeout = (uint64_t)arg4;
	int32_t ret = 0;

	if ((msg != NULL) && (size > 0)) {
		ret = user_buffer_validate(msg, size, BUFFER_PERMIT_READ);
		if (ret == -1) {
			return ERRNO_MSGQ_INVALID_BUFFER;
		}
	}

	return msgq_send(id, msg, size, timeout);
}

static uintptr_t syscall_msgq_receive(uintptr_t arg1, uintptr_t arg2,
									  uintptr_t arg3, uintptr_t arg4,
									  uintptr_t arg5, uintptr_t arg6,
									  struct arch_regs *regs) {
	(void)arg5;
	(void)arg6;
	(void)regs;
	msgq_id_t id = (msgq_id_t)arg1;
	void *msg = (void *)arg2;
	uint32_t *size = (uint32_t *)arg3;
	uint64_t timeout = (uint64_t)arg4;
	int32_t ret = 0;

	if ((msg != NULL) && (size > 0)) {
		ret = user_buffer_validate(msg, *size, BUFFER_PERMIT_WRITE);
		if (ret == -1) {
			return ERRNO_MSGQ_INVALID_BUFFER;
		}
	}

	return msgq_receive(id, msg, size, timeout);
}

static uintptr_t syscall_msgq_destroy(uintptr_t arg1, uintptr_t arg2,
									  uintptr_t arg3, uintptr_t arg4,
									  uintptr_t arg5, uintptr_t arg6,
									  struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	msgq_id_t id = (msgq_id_t)arg1;

	return msgq_destroy(id);
}

static uintptr_t syscall_sem_create(uintptr_t arg1, uintptr_t arg2,
									uintptr_t arg3, uintptr_t arg4,
									uintptr_t arg5, uintptr_t arg6,
									struct arch_regs *regs) {
	(void)arg5;
	(void)arg6;
	(void)regs;
	const char *name = (const char *)arg1;
	uint32_t init_count = (uint32_t)arg2;
	uint32_t max_count = (uint32_t)arg3;
	sem_id_t *id = (sem_id_t *)arg4;

	return sem_create(name, init_count, max_count, id);
}

static uintptr_t syscall_sem_take(uintptr_t arg1, uintptr_t arg2,
								  uintptr_t arg3, uintptr_t arg4,
								  uintptr_t arg5, uintptr_t arg6,
								  struct arch_regs *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	sem_id_t id = (sem_id_t)arg1;
	uint64_t timeout = (uint64_t)arg2;

	return sem_take(id, timeout);
}

static uintptr_t syscall_sem_give(uintptr_t arg1, uintptr_t arg2,
								  uintptr_t arg3, uintptr_t arg4,
								  uintptr_t arg5, uintptr_t arg6,
								  struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	sem_id_t id = (sem_id_t)arg1;

	return sem_give(id);
}

static uintptr_t syscall_sem_destroy(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	sem_id_t id = (sem_id_t)arg1;

	return sem_destroy(id);
}

static uintptr_t syscall_mutex_create(uintptr_t arg1, uintptr_t arg2,
									  uintptr_t arg3, uintptr_t arg4,
									  uintptr_t arg5, uintptr_t arg6,
									  struct arch_regs *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	const char *name = (const char *)arg1;
	mutex_id_t *id = (mutex_id_t *)arg2;

	return mutex_create(name, id);
}

static uintptr_t syscall_mutex_take(uintptr_t arg1, uintptr_t arg2,
									uintptr_t arg3, uintptr_t arg4,
									uintptr_t arg5, uintptr_t arg6,
									struct arch_regs *regs) {
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	mutex_id_t id = (mutex_id_t)arg1;
	uint32_t timeout = (uint32_t)arg2;

	return mutex_take(id, timeout);
}

static uintptr_t syscall_mutex_give(uintptr_t arg1, uintptr_t arg2,
									uintptr_t arg3, uintptr_t arg4,
									uintptr_t arg5, uintptr_t arg6,
									struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	mutex_id_t id = (mutex_id_t)arg1;

	return mutex_give(id);
}

static uintptr_t syscall_mutex_destroy(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	mutex_id_t id = (mutex_id_t)arg1;

	return mutex_destroy(id);
}

static uintptr_t syscall_sched_yield(uintptr_t arg1, uintptr_t arg2,
									 uintptr_t arg3, uintptr_t arg4,
									 uintptr_t arg5, uintptr_t arg6,
									 struct arch_regs *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	return task_delay(0);
}

static uintptr_t syscall_sched_get_priority_max(uintptr_t arg1, uintptr_t arg2,
												uintptr_t arg3, uintptr_t arg4,
												uintptr_t arg5, uintptr_t arg6,
												struct arch_regs *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	return TASK_PRIORITY_HIGHEST;
}

static uintptr_t syscall_sched_get_priority_min(uintptr_t arg1, uintptr_t arg2,
												uintptr_t arg3, uintptr_t arg4,
												uintptr_t arg5, uintptr_t arg6,
												struct arch_regs *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	return TASK_PRIORITY_LOWEST;
}

static uintptr_t syscall_sched_get_param(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 struct arch_regs *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	uint32_t prioriy = 0;

	return -task_priority_get((task_id_t)arg1, &prioriy);
}

static uintptr_t syscall_sched_set_scheduler(uintptr_t arg1, uintptr_t arg2,
											 uintptr_t arg3, uintptr_t arg4,
											 uintptr_t arg5, uintptr_t arg6,
											 struct arch_regs *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	uint32_t prioriy = (uint32_t)arg1;
	
	return -task_priority_set((task_id_t)arg1, prioriy);
}

static uintptr_t default_syscall_handler(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 struct arch_regs *regs) {
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

	return OK;
}

const syscall_handler_t syscall_table[SYSCALL_ID_LIMIT] = {
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
	[SYSCALL_TASK_SCHED_UNLOCK] = syscall_task_sched_unlock,
	[SYSCALL_MSGQ_CREATE] = syscall_msgq_create,
	[SYSCALL_MSGQ_SEND] = syscall_msgq_send,
	[SYSCALL_MSGQ_RECV] = syscall_msgq_receive,
	[SYSCALL_MSGQ_DESTROY] = syscall_msgq_destroy,
	[SYSCALL_SEM_CREATE] = syscall_sem_create,
	[SYSCALL_SEM_TAKE] = syscall_sem_take,
	[SYSCALL_SEM_GIVE] = syscall_sem_give,
	[SYSCALL_SEM_DESTROY] = syscall_sem_destroy,
	[SYSCALL_MUTEX_CREATE] = syscall_mutex_create,
	[SYSCALL_MUTEX_TAKE] = syscall_mutex_take,
	[SYSCALL_MUTEX_GIVE] = syscall_mutex_give,
	[SYSCALL_MUTEX_DESTROY] = syscall_mutex_destroy,
	[SYSCALL_SCHED_YIELD] = syscall_sched_yield,
	[SYSCALL_SCHED_GET_PRIORITY_MAX] = syscall_sched_get_priority_max,
	[SYSCALL_SCHED_GET_PRIORITY_MIN] = syscall_sched_get_priority_min,
	[SYSCALL_SCHED_GET_PARAM] = syscall_sched_get_param,
	[SYSCALL_SCHED_SET_SCHEDULER] = syscall_sched_set_scheduler,
};

void syscall_dispatch(struct arch_regs *regs) {
	syscall_handler_t do_syscall = NULL;
	uintptr_t arg1 = regs->gprs[0];
	uintptr_t arg2 = regs->gprs[1];
	uintptr_t arg3 = regs->gprs[2];
	uintptr_t arg4 = regs->gprs[3];
	uintptr_t arg5 = regs->gprs[4];
	uintptr_t arg6 = regs->gprs[5];
	uint32_t syscall_id = regs->gprs[8];
	ttbr_t ttbr = INVALID_TTBR;

	if (syscall_id >= SYSCALL_ID_LIMIT) {
		regs->gprs[0] = ERRNO_SYSCALL_INVALID_ID;
		return;
	}

	if (syscall_table[syscall_id] == NULL) {
		do_syscall = default_syscall_handler;
	}

	do_syscall = syscall_table[syscall_id];
	ttbr = mem_domain_save();
	regs->gprs[0] = do_syscall(arg1, arg2, arg3, arg4, arg5, arg6, regs);
	mem_domain_restore(ttbr);

	return;
}