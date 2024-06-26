#include <timeout.h>
#include <list.h>
#include <menuconfig.h>
#include <cpu.h>
#include <task_sched.h>
#include <stdio.h>

void timeout_queue_handle(uint64_t cur_ticks) {
	struct list_head *queue = NULL;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct timeout *timeout = NULL;

	queue = &current_percpu_get()->timer_queue.queue;

	list_for_each_safe(pos, next, queue) {
		timeout = list_entry(pos, struct timeout, node);
		if (timeout && timeout->func && cur_ticks >= timeout->deadline_ticks) {
			list_del_init(pos);
			timeout->func(timeout);
		}
	}
}

errno_t timeout_queue_add(struct timeout *timeout) {
	struct list_head *queue = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_ADD_EMPTY_NODE;
	}

	queue = &current_percpu_get()->timer_queue.queue;
	list_add(&timeout->node, queue);

	return OK;
}
