#ifndef __USER_SPACE_H__
#define __USER_SPACE_H__

#include <menuconfig.h>
#include <types.h>
#include <errno.h>
#include <mem_domain.h>
#include <app_mem_domain.h>
#include <task.h>

#ifdef CONFIG_USER_SPACE

/* user log module*/
bool user_log_level_set(int32_t level);
int32_t user_log_level_get();

/* user task module */
errno_t user_task_create(task_id_t *task_id, const char *name,
						 task_entry_func entry, void *arg0, void *arg1,
						 void *arg2, void *arg3, struct task *task, void *stack,
						 uint32_t stack_size, uint32_t flag);
errno_t user_task_start(task_id_t task_id);
errno_t user_task_stop(task_id_t task_id);
errno_t user_task_delay(uint64_t ticks);
errno_t user_task_suspend(task_id_t task_id);
errno_t user_task_resume(task_id_t task_id);
errno_t user_task_priority_set(task_id_t task_id, uint32_t prioriy);
errno_t user_task_priority_get(task_id_t task_id, uint32_t *prioriy);
errno_t user_task_self_id(task_id_t *task_id);

/* user task sched module */
errno_t user_task_sched_unlock();

#endif
#endif
