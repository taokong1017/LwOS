#include <sched.h>
#include <arch_syscall.h>

int sched_setparam(pid_t pid, const struct sched_param *param) {
	int ret = 0;

	if (!param) {
		return -1;
	}

	ret = arch_syscall_invoke2(pid, param->sched_priority,
							   SYSCALL_SCHED_SET_PARAM);
	if (ret < 0) {
		return -1;
	}

	return 0;
}
