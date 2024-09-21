#include <arch_syscall.h>
#include <syscall.h>

bool syscall_log_level_set(int32_t level) {
	return arch_syscall_invoke1(level, SYSCALL_LOG_LEVEL_SET);
}

int32_t syscall_log_level_get() {
	return arch_syscall_invoke0(SYSCALL_LOG_LEVEL_GET);
}
