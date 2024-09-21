#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <types.h>

enum syscall_id {
	SYSCALL_LOG_LEVEL_GET = 0,
	SYSCALL_LOG_LEVEL_SET = 1,
	SYSCALL_ID_LIMIT,
};

#endif
