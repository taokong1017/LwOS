#include <syscall.h>
#include <log.h>
#include <compiler.h>

#define SYSCALL_TAG "SYSCALL"

static uintptr_t default_handler_syscall(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 void *ssf) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	uintptr_t syscall_id = arg1;

	log_info(SYSCALL_TAG, "System call id %lu is invoked\n", syscall_id);
	forever();
	code_unreachable();
}

const syscall_handler_t syscall_table[SYSCALL_ID_LIMIT] = {
	[SYSCALL_LOG_LEVEL_GET] = default_handler_syscall,
	[SYSCALL_LOG_LEVEL_SET] = default_handler_syscall,
};
