#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <types.h>
#include <list.h>
#include <errno.h>

#define ERRNO_TIMEOUT_EMPTY_PTR ERRNO_OS_ERROR(MOD_ID_TIMEOUT, 0x00)

struct timeout;
typedef bool (*timeout_func)(struct timeout *timeout);

struct timer_queue {
	struct list_head queue;
};

struct timeout {
	struct list_head node;
	timeout_func func;
	uint64_t deadline_ticks;
};

void timeout_queue_handle(uint64_t cur_ticks);
errno_t timeout_queue_add(struct timeout *timeout, uint32_t cpu_id);
errno_t timeout_queue_del(struct timeout *timeout);

#endif
