#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <types.h>
#include <list.h>

struct timeout;
typedef void (*timeout_func)(struct timeout *timeout);

struct timer_queue {
	struct list_head tq;
};

struct timeout {
	struct list_head node;
	timeout_func timeout;
	uint64_t deadline_ticks;
};

#endif
