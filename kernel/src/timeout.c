#include <timeout.h>
#include <list.h>
#include <menuconfig.h>
#include <cpu.h>
#include <task_sched.h>
#include <stdio.h>
#include <smp.h>

SPIN_LOCK_DECLARE(sched_spinlocker);

void timeout_queue_handle(uint64_t cur_ticks) {
	struct list_head *queue = NULL;
	struct timeout *timeout = NULL, *next = NULL;
	bool need_sched = false;
	struct per_cpu *per_cpu = current_percpu_get();
	uint32_t key = sched_spin_lock();

	queue = &per_cpu->timer_queue.queue;
	if (list_empty(queue)) {
		sched_spin_unlock(key);
		return;
	}
	list_for_each_entry_safe(timeout, next, queue, node) {
		if (timeout && cur_ticks >= timeout->deadline_ticks) {
			list_del_init(&timeout->node);
			if (timeout->func) {
				need_sched |= timeout->func(timeout);
			}
		}
	}

	if (need_sched) {
		per_cpu->pend_sched = true;
		smp_sched_notify();
	}

	sched_spin_unlock(key);
}

errno_t timeout_queue_add(struct timeout *timeout, uint32_t cpu_id) {
	struct list_head *queue = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_EMPTY_PTR;
	}

	queue = &percpu_get(cpu_id)->timer_queue.queue;
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
