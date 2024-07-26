#include <timeout.h>
#include <list.h>
#include <menuconfig.h>
#include <cpu.h>
#include <task_sched.h>
#include <stdio.h>

void timeout_queue_handle(uint64_t cur_ticks) {
	struct list_head *queue = NULL;
	struct timeout *timeout = NULL, *next = NULL;

	queue = &current_percpu_get()->timer_queue.queue;
	list_for_each_entry_safe(timeout, next, queue, node) {
		if (timeout && timeout->func && cur_ticks >= timeout->deadline_ticks) {
			list_del_init(&timeout->node);
			timeout->func(timeout);
		}
	}
}

errno_t timeout_queue_add(struct timeout *timeout) {
	struct list_head *queue = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_EMPTY_PTR;
	}

	queue = &current_percpu_get()->timer_queue.queue;
	list_add_tail(&timeout->node, queue);

	return OK;
}

errno_t timeout_queue_del(struct timeout *timeout) {
	struct list_head *queue = NULL;
	struct timeout *tmp_timeout = NULL, *next = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_EMPTY_PTR;
	}

	queue = &current_percpu_get()->timer_queue.queue;
	list_for_each_entry_safe(tmp_timeout, next, queue, node) {
		if (tmp_timeout == timeout) {
			list_del_init(&tmp_timeout->node);
			break;
		}
	}

	return OK;
}
