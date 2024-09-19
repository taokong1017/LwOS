#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>

enum syscall_id {
	SYSCALL_LOG_LEVEL_GET = 0,
	SYSCALL_LOG_LEVEL_SET = 1,
	SYSCALL_ID_LIMIT,
};

typedef uintptr_t (*syscall_handler_t)(uintptr_t arg1, uintptr_t arg2,
									   uintptr_t arg3, uintptr_t arg4,
									   uintptr_t arg5, uintptr_t arg6,
									   void *ssf);

#endif
