#ifndef __USER_SPACE_H__
#define __USER_SPACE_H__

#include <menuconfig.h>
#include <types.h>
#include <kerrno.h>
#include <mem_domain.h>
#include <user_mem_domain.h>
#include <task.h>
#include <msgq.h>
#include <sem.h>
#include <mutex.h>

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

/* user message queue module */
errno_t user_msgq_create(const char *name, uint32_t max_msgs, uint32_t msg_size,
						 msgq_id_t *id);
errno_t user_msgq_send(msgq_id_t id, const void *msg, uint32_t size,
					   uint64_t timeout);
errno_t user_msgq_receive(msgq_id_t id, void *msg, uint32_t *size,
						  uint64_t timeout);
errno_t user_msgq_destroy(msgq_id_t id);

/* user semaphore module */
errno_t user_sem_create(const char *name, uint32_t count, uint32_t max_count,
						sem_id_t *id);
errno_t user_sem_take(sem_id_t id, uint64_t timeout);
errno_t user_sem_give(sem_id_t id);
errno_t user_sem_destroy(sem_id_t id);

/* user mutex module */
errno_t user_mutex_create(const char *name, mutex_id_t *id);
errno_t user_mutex_take(mutex_id_t id, uint32_t timeout);
errno_t user_mutex_give(mutex_id_t id);
errno_t user_mutex_destroy(mutex_id_t id);

#endif
#endif
