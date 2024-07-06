#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <types.h>
#include <errno.h>
#include <task.h>

#define MUTEX_INVALID_ID -1
#define MUTEX_NAME_LEN 32
#define MUTEX_WAIT_FOREVER WAIT_FOREVER
#define MUTEX_NO_WAIT NO_WAIT
#define MUTEX_CEILING_PRIORITY (TASK_PRIORITY_HIGHEST + 1)

typedef long mutex_id_t;

struct mutex {
	mutex_id_t id;			   /* mutex id */
	char name[MUTEX_NAME_LEN]; /* mutex name */

	struct task *owner;	 /* owner task */
	uint32_t lock_count; /* mutex lock count */
	uint32_t priority;	 /* the origion priority */

	struct wait_queue wait_queue; /* pending queue */
};

errno_t mutex_create(const char *name, mutex_id_t *id);
errno_t mutex_destroy(mutex_id_t id);
errno_t mutex_take(mutex_id_t id, uint32_t timeout);
errno_t mutex_give(mutex_id_t id);

#endif
