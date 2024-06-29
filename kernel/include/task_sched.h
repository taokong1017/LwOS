#ifndef __TASK_SCHED_H__
#define __TASK_SCHED_H__

#include <kernel.h>
#include <spin_lock.h>

#define forever() for (;;)

void task_sched_init();
uint32_t sched_spin_lock();
void sched_spin_unlock(uint32_t key);
void sched_ready_queue_remove(uint32_t cpu_id, struct task *task);
void sched_ready_queue_add(uint32_t cpu_id, struct task *task);
struct task *current_task_get();
void task_locked_sched();
bool is_in_irq();
struct per_cpu *current_percpu_get();

#endif
