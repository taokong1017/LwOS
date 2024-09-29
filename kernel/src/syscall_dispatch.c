#include <syscall.h>
#include <log.h>
#include <compiler.h>
#include <task.h>
#include <percpu.h>
#include <task_sched.h>

#define SYSCALL_TAG "SYSCALL"
typedef uintptr_t (*syscall_handler_t)(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs);

static uintptr_t syscall_log_level_set(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;
	int32_t level = (int32_t)arg1;

	return log_level_set(level);
}

static uintptr_t syscall_log_level_get(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	return log_level_get();
}

static uintptr_t default_syscall_handler(uintptr_t arg1, uintptr_t arg2,
										 uintptr_t arg3, uintptr_t arg4,
										 uintptr_t arg5, uintptr_t arg6,
										 void *regs) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	(void)arg4;
	(void)arg5;
	(void)arg6;
	(void)regs;

	log_info(SYSCALL_TAG, "enter default syscall handler\n");
	forever();
	code_unreachable();
}

const syscall_handler_t syscall_table[SYSCALL_ID_LIMIT + 1] = {
	[SYSCALL_LOG_LEVEL_GET] = syscall_log_level_get,
	[SYSCALL_LOG_LEVEL_SET] = syscall_log_level_set,
	[SYSCALL_ID_LIMIT] = default_syscall_handler,
};

uint32_t syscall_table_size_get() { return ARRAY_SIZE(syscall_table); }
