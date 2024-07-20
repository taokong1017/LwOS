#ifndef __TASK_SCHED_H__
#define __TASK_SCHED_H__

#include <kernel.h>
#include <spin_lock.h>

void task_sched_init();
uint32_t sched_spin_lock();
void sched_spin_unlock(uint32_t key);
void sched_ready_queue_remove(uint32_t cpu_id, struct task *task);
void sched_ready_queue_add(uint32_t cpu_id, struct task *task);
struct task *current_task_get();
void task_sched_locked();
void task_sched_unlocked();
bool is_in_irq();

#endif
