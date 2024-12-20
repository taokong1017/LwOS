#ifndef __TASK_SCHED_H__
#define __TASK_SCHED_H__

#include <kernel.h>
#include <spin_lock.h>

uint32_t sched_spin_lock();
void sched_spin_unlock(uint32_t key);
bool sched_spin_is_locked();
void sched_ready_queue_remove(uint32_t cpu_id, struct task *task);
void sched_ready_queue_add(uint32_t cpu_id, struct task *task);
void sched_ready_queue_dump(uint32_t cpu_id);
struct task *current_task_get();
void task_sched_locked();
void task_sched_unlocked();
bool is_in_irq();
void idle_task_create(uint32_t cpu_id);
void main_task_create(uint32_t cpu_id);
void system_task_create(uint32_t cpu_id);
void task_sched_start();
void sched_spin_lock_dump();
errno_t task_sched_unlock();

#endif
