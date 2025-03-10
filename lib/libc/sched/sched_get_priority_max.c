#include <sched.h>
#include <arch_syscall.h>

int sched_get_priority_max(int policy) {
	if (policy == SCHED_FIFO) {
		return arch_syscall_invoke0(SYSCALL_SCHED_GET_PRIORITY_MAX);
	}

	return -1;
}

int sched_get_priority_min(int policy) {
	if (policy == SCHED_FIFO) {
		return arch_syscall_invoke0(SYSCALL_SCHED_GET_PRIORITY_MIN);
	}

	return -1;
}
