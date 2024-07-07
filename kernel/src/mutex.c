#include <mutex.h>
#include <memory.h>
#include <log.h>
#include <string.h>
#include <task_sched.h>

#define MUTEX_TAG "MUTEX"
#define mutex2id(mutex) ((mutex_id_t)mutex)
#define id2mutex(id) ((struct mutex *)id)
#define TASK_SCHED_LOCKED(task) (task->lock_cnt > 1)

errno_t mutex_create(const char *name, mutex_id_t *id) {
	struct mutex *mutex = NULL;

	if (!name) {
		log_err(MUTEX_TAG, "the name is empty\n");
		return ERRNO_MUTEX_NAME_EMPTY;
	}

	if (!id) {
		log_err(MUTEX_TAG, "the id is null pointer\n");
		return ERRNO_MUTEX_PTR_NULL;
	}

	mutex = (struct mutex *)mem_alloc_align(sizeof(struct mutex),
											MEM_DEFAULT_ALIGN);
	if (!mutex) {
		log_fatal(MUTEX_TAG, "allocate the mutex %s failed without memory\n",
				  name);
		return ERRNO_MUTEX_NO_MEMORY;
	}

	mutex->id = mutex2id(mutex);
	strncpy(mutex->name, name, MUTEX_NAME_LEN);
	mutex->owner = NULL;
	mutex->lock_count = 0;
	mutex->priority = 0;
	INIT_LIST_HEAD(&mutex->wait_queue.wait_list);
	*id = mutex->id;

	return OK;
}

errno_t mutex_destroy(mutex_id_t id) {
	struct mutex *mutex = id2mutex(id);
	uint32_t key = 0;

	if (!mutex || mutex->id != id) {
		log_err(MUTEX_TAG, "the id is null pointer\n");
		return ERRNO_MUTEX_INVALID_ID;
	}

	key = sched_spin_lock();
	if (mutex->lock_count > 0 || !list_empty(&mutex->wait_queue.wait_list)) {
		log_err(MUTEX_TAG, "the mutex %s is locked\n", mutex->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IS_BUSY;
	}

	mutex->id = MUTEX_INVALID_ID;
	log_debug(MUTEX_TAG, "the mutex %s is destroyed\n", mutex->name);
	mem_free(mutex);

	sched_spin_unlock(key);

	return OK;
}

static void mutex_owner_priority_set(struct task *task, uint32_t priority) {
	if (task->status == TASK_STATUS_READY) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->priority = priority;
		sched_ready_queue_add(task->cpu_id, task);
	} else {
		task->priority = priority;
	}
}

errno_t mutex_take(mutex_id_t id, uint32_t timeout) {
	struct mutex *mutex = id2mutex(id);
	uint32_t key = 0;
	struct task *cur_task = NULL;
	struct task *pos = NULL;
	uint32_t cur_prio = 0;
	uint32_t priority = 0;

	if (!mutex || mutex->id != id) {
		log_err(MUTEX_TAG, "the id is null pointer\n");
		return ERRNO_MUTEX_INVALID_ID;
	}

	key = sched_spin_lock();

	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IN_IRQ;
	}

	cur_task = current_task_get();
	cur_prio = cur_task->priority;

	/* current task take the mutex */
	if (!mutex->lock_count || mutex->owner == cur_task) {
		if (mutex->lock_count == 0) {
			mutex->priority = cur_prio;
			mutex->owner = cur_task;
		}
		mutex->lock_count++;
		log_debug(MUTEX_TAG,
				  "mutex %s is taken by current task %s, lock count is %u\n",
				  mutex->name, cur_task->name, mutex->lock_count);
		sched_spin_unlock(key);
		return OK;
	}

	/* other task takes the mutex */
	if (timeout == MUTEX_NO_WAIT) {
		log_err(MUTEX_TAG, "the mutex %s is locked by task %s\n", mutex->name,
				mutex->owner->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IS_BUSY;
	}

	if (TASK_SCHED_LOCKED(cur_task)) {
		log_debug(MUTEX_TAG, "the task %s is locked\n", cur_task->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IS_LOCKED;
	}

	/* raise mutex owner priority to the hightest of tasks in the wait queue */
	if (mutex->owner->priority < cur_prio) {
		mutex_owner_priority_set(mutex->owner, cur_prio);
		log_debug(
			MUTEX_TAG,
			"raise the priority of the task %s to %u, and current task is %s",
			mutex->owner->name, cur_prio, cur_task->name);
	}

	/* current task is wakeup with timeout */
	if (task_wait_locked(&mutex->wait_queue, timeout, true)) {
		log_debug(MUTEX_TAG, "the task %s is wakeup with timeout\n",
				  cur_task->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_TIMEOUT;
	}

	/* current task is wakeup and take a mutex */
	if (mutex->owner /* current task */) {
		/* get the highest priority of tasks in the wait queue, and set it to
		 * mutex owner */
		list_for_each_entry(pos, &mutex->wait_queue.wait_list, pend_list) {
			if (pos->priority > priority) {
				priority = pos->priority;
			}
		}
		mutex_owner_priority_set(mutex->owner, priority);
		log_debug(
			MUTEX_TAG,
			"raise the priority of the task %s to %u, and current task is %s",
			mutex->owner->name, priority, cur_task->name);
	}

	sched_spin_unlock(key);

	return OK;
}

errno_t mutex_give(mutex_id_t id) {
	struct mutex *mutex = id2mutex(id);
	uint32_t key = 0;
	struct task *owner = NULL;
	struct task *cur_task = NULL;

	if (!mutex || mutex->id != id) {
		log_err(MUTEX_TAG, "the id is null pointer\n");
		return ERRNO_MUTEX_INVALID_ID;
	}

	key = sched_spin_lock();

	if (is_in_irq()) {
		log_err(MUTEX_TAG, "the mutex %s is in irq\n", mutex->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IN_IRQ;
	}

	owner = mutex->owner;
	if (!owner) {
		log_err(MUTEX_TAG, "the mutex %s has no owner\n", mutex->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_WITHOUT_OWNER;
	}

	cur_task = current_task_get();
	if (owner != cur_task) {
		log_err(MUTEX_TAG,
				"the mutex %s is owned by other(not current) task %s\n",
				mutex->name, owner->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_OHTER_OWNER;
	}

	if (TASK_SCHED_LOCKED(cur_task)) {
		log_debug(MUTEX_TAG, "the task %s is locked\n", cur_task->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_IS_LOCKED;
	}

	if (mutex->lock_count > 1U) {
		mutex->lock_count--;
		log_debug(MUTEX_TAG, "lock count of mutex %s is %u\n", mutex->name,
				  mutex->lock_count);
		sched_spin_unlock(key);
		return OK;
	}

	/* restore the priority of mutex owner, and needs to adjust queue because
	 * the priority is lowered */
	mutex_owner_priority_set(owner /* current task */, mutex->priority);

	/* wakeup the first task waiting for the mutex */
	if (!list_empty(&mutex->wait_queue.wait_list)) {
		mutex->owner = list_first_entry(&mutex->wait_queue.wait_list,
										struct task, pend_list);
		mutex->lock_count = 1;
		mutex->priority = mutex->owner->priority;
		task_wakeup_locked(&mutex->wait_queue);
	} else {
		task_sched_locked();
	}
	sched_spin_unlock(key);

	return OK;
}
