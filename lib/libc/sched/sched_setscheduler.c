#include <sched.h>
#include <arch_syscall.h>

int sched_setscheduler(pid_t pid, int sched, const struct sched_param *param) {
	int ret = 0;

	if ((sched != SCHED_FIFO) || !param) {
		return -1;
	}

	ret = arch_syscall_invoke2(pid, param->sched_priority,
							   SYSCALL_SCHED_SET_SCHEDULER);
	if (ret < 0) {
		return -1;
	}

	return 0;
}
