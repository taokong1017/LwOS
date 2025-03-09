#include <sched.h>
#include <arch_syscall.h>

int sched_yield()
{
	return arch_syscall_invoke0(SYSCALL_LIBC_SCHED_YIELD);
}
