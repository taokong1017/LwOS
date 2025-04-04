#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>
#include <kerrno.h>

#define ERRNO_SYSCALL_INVALID_ID ERRNO_OS_ERROR(MOD_ID_SYSCALL, 0x00)

enum syscall_id {
	SYSCALL_LOG_LEVEL_GET = 0,
	SYSCALL_LOG_LEVEL_SET,
	SYSCALL_TASK_CREATE,
	SYSCALL_TASK_START,
	SYSCALL_TASK_STOP,
	SYSCALL_TASK_DELAY,
	SYSCALL_TASK_SUSPEND,
	SYSCALL_TASK_RESUME,
	SYSCALL_TASK_PRIO_SET,
	SYSCALL_TASK_PRIO_GET,
	SYSCALL_TASK_SELF_ID,
	SYSCALL_TASK_SCHED_UNLOCK,
	SYSCALL_MSGQ_CREATE,
	SYSCALL_MSGQ_SEND,
	SYSCALL_MSGQ_RECV,
	SYSCALL_MSGQ_DESTROY,
	SYSCALL_SEM_CREATE,
	SYSCALL_SEM_TAKE,
	SYSCALL_SEM_GIVE,
	SYSCALL_SEM_DESTROY,
	SYSCALL_SEM_VALUE_GET,
	SYSCALL_MUTEX_CREATE,
	SYSCALL_MUTEX_TAKE,
	SYSCALL_MUTEX_GIVE,
	SYSCALL_MUTEX_DESTROY,
	SYSCALL_SCHED_YIELD,
	SYSCALL_SCHED_GET_PRIORITY_MAX,
	SYSCALL_SCHED_GET_PRIORITY_MIN,
	SYSCALL_SCHED_GET_PARAM,
	SYSCALL_SCHED_SET_SCHEDULER,
	SYSCALL_SCHED_SET_PARAM,
	SYSCALL_SCHED_GETCPU,
	SYSCALL_SCHED_SET_AFFINITY,
	SYSCALL_SCHED_GET_AFFINITY,
	SYSCALL_ID_LIMIT,
};

struct task_info {
	const char *name;
	struct task *task;
	void *entry;
	void *arg0;
	void *arg1;
	void *arg2;
	void *arg3;
	void *stack;
	uint32_t stack_size;
	uint32_t flag;
};

#endif
