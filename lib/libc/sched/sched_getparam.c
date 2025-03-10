#include <sched.h>
#include <arch_syscall.h>

int sched_getparam(pid_t pid, struct sched_param *param) {
	int ret = 0;

	if (!param) {
		return -1;
	}

	ret = arch_syscall_invoke1(pid, SYSCALL_SCHED_GET_PARAM);
	if (ret < 0) {
		return -1;
	}

	param->sched_priority = ret;

	return 0;
}
