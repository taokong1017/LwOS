#include <sched.h>
#include <arch_syscall.h>
#include <string.h>

int sched_setaffinity(pid_t tid, size_t size, const cpu_set_t *set) {
	int ret = arch_syscall_invoke3(tid, size, (uintptr_t)set,
								   SYSCALL_SCHED_SET_AFFINITY);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int sched_getaffinity(pid_t tid, size_t size, cpu_set_t *set) {
	long ret = arch_syscall_invoke3(tid, size, (uintptr_t)set,
									SYSCALL_SCHED_GET_AFFINITY);

	if (ret < 0) {
		return -1;
	}

	if (ret < size) {
		memset((char *)set + ret, 0, size - ret);
	}

	return 0;
}
