#include <sched.h>
#include <arch_syscall.h>

int sched_getcpu(void) {
	int r; 
	unsigned int cpu;

	r = arch_syscall_invoke1((uintptr_t)&cpu, SYSCALL_SCHED_GETCPU);

	return (r < 0) ? -1 : cpu;
}
