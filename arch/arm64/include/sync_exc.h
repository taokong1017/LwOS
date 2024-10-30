#ifndef __ARM64_SYNC_EXC_H__
#define __ARM64_SYNC_EXC_H__

#include <types.h>

enum buffer_permit {
	BUFFER_PERMIT_READ = 0,
	BUFFER_PERMIT_WRITE = 1,
};

int32_t user_buffer_validate(const void *addr, size_t size,
							 enum buffer_permit permit);

#endif
