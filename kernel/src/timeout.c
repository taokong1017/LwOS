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
	struct per_cpu *per_cpu = current_percpu_get();
	uint32_t key = sched_spin_lock();

	queue = &per_cpu->timer_queue.queue;
	list_for_each_entry_safe(timeout, next, queue, node) {
		if (timeout && cur_ticks >= timeout->deadline_ticks) {
			timeout_queue_del(timeout, arch_cpu_id_get());
			timeout->func(timeout);
		}
	}

	sched_spin_unlock(key);
}

errno_t timeout_queue_add(struct timeout *timeout, uint32_t cpu_id) {
	struct list_head *queue = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_EMPTY_PTR;
	}

	timeout_queue_del(timeout, cpu_id);
	queue = &percpu_get(cpu_id)->timer_queue.queue;
	list_add_tail(&timeout->node, queue);

	return OK;
}

errno_t timeout_queue_del(struct timeout *timeout, uint32_t cpu_id) {
	struct list_head *queue = NULL;
	struct timeout *tmp_timeout = NULL, *next = NULL;

	if (!timeout) {
		return ERRNO_TIMEOUT_EMPTY_PTR;
	}

	queue = &percpu_get(cpu_id)->timer_queue.queue;
	list_for_each_entry_safe(tmp_timeout, next, queue, node) {
		if (tmp_timeout == timeout) {
			list_del_init(&tmp_timeout->node);
			break;
		}
	}

	return OK;
}
