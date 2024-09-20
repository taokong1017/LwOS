#include <syscall.h>
#include <log.h>
#include <compiler.h>

#define SYSCALL_TAG "SYSCALL"
#define array_size(x) (sizeof(x) / sizeof((x)[0]))

static uintptr_t default_handler_syscall(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	uintptr_t syscall_id = arg1;

	log_info(SYSCALL_TAG, "System call id %lu is invoked\n", syscall_id);
	forever();
	code_unreachable();
}

const syscall_handler_t syscall_table[SYSCALL_ID_LIMIT + 1] = {
	[SYSCALL_LOG_LEVEL_GET] = default_handler_syscall,
	[SYSCALL_LOG_LEVEL_SET] = default_handler_syscall,
	[SYSCALL_ID_LIMIT] = default_handler_syscall};

uint32_t syscall_table_size_get() { return array_size(syscall_table); }
