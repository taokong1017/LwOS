#ifndef __TASK_SCHED_H__
#define __TASK_SCHED_H__

#include <kernel.h>
#include <spin_lock.h>

#define forever() for (;;)
#define do_sched_spin_lock()                                                   \
	for (struct spinlock_key i = {0}, lock_key = sched_spin_lock(); !i.key;    \
		 sched_spin_unlock(lock_key), i.key = 1)

void task_sched_init();
struct spinlock_key sched_spin_lock();
void sched_spin_unlock(struct spinlock_key key);
void sched_ready_queue_remove(uint32_t cpu_id, struct task *task);
void sched_ready_queue_add(uint32_t cpu_id, struct task *task);
struct task *current_task_get();
void task_resched();
bool is_in_irq();
struct per_cpu* current_percpu_get();

#endif
