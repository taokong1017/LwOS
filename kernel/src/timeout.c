#include <timeout.h>
#include <list.h>
#include <menuconfig.h>
#include <spin_lock.h>
#include <cpu.h>
#include <task_sched.h>
#include <stdio.h>

static struct spinlock timeout_locks[CONFIG_CPUS_MAX_NUM] = {{0}};

void timeout_queue_handle(uint64_t cur_ticks) {
	struct list_head *queue = NULL;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct timeout *timeout = NULL;
	uint32_t cpu_id = arch_cpu_id_get();
	struct spinlock *spinlock = &timeout_locks[cpu_id];

	spin_lock(spinlock);
	queue = &current_percpu_get()->timer_queue.queue;

	list_for_each_safe(pos, next, queue) {
		timeout = list_entry(pos, struct timeout, node);
		if (timeout && timeout->func && cur_ticks >= timeout->deadline_ticks) {
			list_del_init(pos);
			timeout->func(timeout);
		}
	}

	spin_unlock(spinlock);
}

errno_t timeout_queue_add(struct timeout *timeout) {
	struct list_head *queue = NULL;
	uint32_t cpu_id = arch_cpu_id_get();
	struct spinlock *spinlock = &timeout_locks[cpu_id];

	if (!timeout) {
		return ERRNO_TIMEOUT_ADD_EMPTY_NODE;
	}

	spin_lock(spinlock);
	queue = &current_percpu_get()->timer_queue.queue;
	list_add(&timeout->node, queue);
	spin_unlock(spinlock);

	return OK;
}
