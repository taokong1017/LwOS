#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <types.h>
#include <errno.h>
#include <task.h>
#include <kernel.h>

#define MUTEX_INVALID_ID -1
#define MUTEX_NAME_LEN 32
#define MUTEX_WAIT_FOREVER WAIT_FOREVER
#define MUTEX_NO_WAIT NO_WAIT

/* sem error code definition */
#define ERRNO_MUTEX_NAME_EMPTY ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x00)
#define ERRNO_MUTEX_PTR_NULL ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x01)
#define ERRNO_MUTEX_INVALID_ID ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x02)
#define ERRNO_MUTEX_IS_BUSY ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x03)
#define ERRNO_MUTEX_IN_IRQ ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x04)
#define ERRNO_MUTEX_IS_LOCKED ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x05)
#define ERRNO_MUTEX_WITHOUT_OWNER ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x06)
#define ERRNO_MUTEX_OHTER_OWNER ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x07)
#define ERRNO_MUTEX_TIMEOUT ERRNO_OS_ERROR(MOD_ID_MUTEX, 0x08)
#define ERRNO_MUTEX_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_MUTEX, 0x09)

typedef long mutex_id_t;

struct mutex {
	mutex_id_t id;			   /* mutex id */
	char name[MUTEX_NAME_LEN]; /* mutex name */

	struct task *owner;	 /* owner task */
	uint32_t lock_count; /* mutex lock count */
	uint32_t priority;	 /* the origion priority */

	struct wait_queue wait_queue; /* pending queue */
};

errno_t mutex_init(struct mutex *mutex, const char *name);
errno_t mutex_create(const char *name, mutex_id_t *id);
errno_t mutex_destroy(mutex_id_t id);
errno_t mutex_take(mutex_id_t id, uint32_t timeout);
errno_t mutex_give(mutex_id_t id);

#endif
