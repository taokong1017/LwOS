#ifndef __LOS_ERRNO_H__
#define __LOS_ERRNO_H__

#include <types.h>

#define OK 0
#define ERRNO_OS_ID (0x01U << 16)

#define ERRTYPE_NORMAL (0x01U << 24)
#define ERRTYPE_WARN (0x02U << 24)
#define ERRTYPE_ERROR (0x03U << 24)
#define ERRTYPE_FATAL (0x04U << 24)

#define ERRNO_OS_FATAL(mID, errno)                                             \
	(ERRTYPE_FATAL | ERRNO_OS_ID | ((uint32_t)(mID) << 8) | ((uint32_t)(errno)))
#define ERRNO_OS_ERROR(mID, errno)                                             \
	(ERRTYPE_ERROR | ERRNO_OS_ID | ((uint32_t)(mID) << 8) | ((uint32_t)(errno)))
#define ERRNO_OS_WARN(mID, errno)                                              \
	(ERRTYPE_WARN | ERRNO_OS_ID | ((uint32_t)(mID) << 8) | ((uint32_t)(errno)))
#define ERRNO_OS_NORMAL(MID, ERRNO)                                            \
	(ERRTYPE_NORMAL | ERRNO_OS_ID | ((uint32_t)(mID) << 8) |                   \
	 ((uint32_t)(errno)))

enum MOUDLE_ID {
	MOD_ID_TASK = 0x1,
	MOD_ID_SPINLOCK,
	MOD_ID_TIMEOUT,
};

typedef uint32_t errno_t;

#endif
