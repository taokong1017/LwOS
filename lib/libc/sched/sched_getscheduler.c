#include <sched.h>
#include <arch_syscall.h>

int sched_getscheduler(pid_t pid) {
	(void)pid;

	return SCHED_FIFO;
}
