#include <mutex.h>
#include <mem_mgr.h>
#include <log.h>
#include <string.h>
#include <task_sched.h>

#define MUTEX_TAG "MUTEX"
#define mutex2id(mutex) ((mutex_id_t)mutex)
#define id2mutex(id) ((struct mutex *)id)

errno_t mutex_init(struct mutex *mutex, const char *name) {
	if (!mutex) {
		log_err(MUTEX_TAG, "the mutex is null pointer\n");
		return ERRNO_MUTEX_PTR_NULL;
	}

	if (!name) {
		log_err(MUTEX_TAG, "the name is empty\n");
		return ERRNO_MUTEX_NAME_EMPTY;
	}

	mutex->id = mutex2id(mutex);
	strncpy(mutex->name, name, MUTEX_NAME_LEN);
	mutex->name[MUTEX_NAME_LEN - 1] = '\0';
	mutex->owner = NULL;
	mutex->lock_count = 0;
	mutex->priority = 0;
	INIT_LIST_HEAD(&mutex->wait_queue.wait_list);

	return OK;
}

errno_t mutex_create(const char *name, mutex_id_t *id) {
	struct mutex *mutex = NULL;
	errno_t errno = OK;

	if (!name) {
		log_err(MUTEX_TAG, "the name is empty\n");
		return ERRNO_MUTEX_NAME_EMPTY;
	}

	if (!id) {
		log_err(MUTEX_TAG, "the id is null pointer\n");
		return ERRNO_MUTEX_PTR_NULL;
	}

	mutex = (struct mutex *)kmalloc(sizeof(struct mutex));
	if (!mutex) {
		log_fatal(MUTEX_TAG, "allocate the mutex %s failed without memory\n",
				  name);
		return ERRNO_MUTEX_NO_MEMORY;
	}

	errno = mutex_init(mutex, name);
	if (errno != OK) {
		log_fatal(MUTEX_TAG, "init the mutex %s failed\n", name);
		return errno;
	}

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
	kfree(mutex);

	sched_spin_unlock(key);

	return OK;
}

static void mutex_owner_priority_set(struct task *task, uint32_t priority) {
	if (TASK_IS_READY(task) || TASK_IS_RUNNING(task)) {
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
	struct task *pend_task = NULL;
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

	/* raise mutex owner priority to the hightest of tasks in the wait queue */
	if (mutex->owner->priority < cur_prio) {
		mutex_owner_priority_set(mutex->owner, cur_prio);
		log_debug(
			MUTEX_TAG,
			"raise the priority of the task %s from %u to %u, and current task is %s\n",
			mutex->owner->name, mutex->owner->priority, cur_prio,
			cur_task->name);
	}

	log_debug(MUTEX_TAG, "task %s waits for mutex %s\n", cur_task->name,
			  mutex->name);
	/* current task is wakeup with timeout */
	if (task_wait_locked(&mutex->wait_queue, timeout, true)) {
		log_debug(MUTEX_TAG, "task %s is wakeup with timeout\n",
				  cur_task->name);
		sched_spin_unlock(key);
		return ERRNO_MUTEX_TIMEOUT;
	}

	/* raise mutex owner priority to the hightest of tasks in the wait queue */
	priority = mutex->priority;
	if (!list_empty(&mutex->wait_queue.wait_list)) {
		pend_task = list_first_entry(&mutex->wait_queue.wait_list, struct task,
									 task_node);
		if (pend_task->priority > priority) {
			priority = pend_task->priority;
		}
	}

	if (priority > cur_task->priority) {
		mutex_owner_priority_set(mutex->owner, priority);
		log_debug(MUTEX_TAG,
				  "raise the priority of the task %s from %u to %u\n",
				  mutex->owner->name, mutex->owner->priority, priority);
	}

	log_debug(MUTEX_TAG, "task %s is woken up\n", cur_task->name);
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
		log_debug(MUTEX_TAG, "the mutex %s is in irq\n", mutex->name);
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

	if (mutex->lock_count > 1U) {
		mutex->lock_count--;
		log_debug(MUTEX_TAG, "lock count of mutex %s is %u\n", mutex->name,
				  mutex->lock_count);
		sched_spin_unlock(key);
		return OK;
	}

	if (owner->priority > mutex->priority) {
		log_debug(MUTEX_TAG,
				  "set mutex owner %s(pri:%u status: %u) priority to %u\n",
				  owner->name, owner->priority, owner->status, mutex->priority);
		mutex_owner_priority_set(owner, mutex->priority);
	}

	if (list_empty(&mutex->wait_queue.wait_list)) {
		mutex->lock_count = 0;
		mutex->owner = NULL;
		mutex->priority = 0;
		sched_spin_unlock(key);
		return OK;
	}

	mutex->owner =
		list_first_entry(&mutex->wait_queue.wait_list, struct task, task_node);
	mutex->lock_count = 1;
	mutex->priority = mutex->owner->priority;
	log_debug(MUTEX_TAG, "wakeup mutex owner %s(pri:%u) by task %s(pri:%u)\n",
			  mutex->owner->name, mutex->priority, cur_task->name,
			  cur_task->priority);
	task_wakeup_locked(&mutex->wait_queue);
	sched_spin_unlock(key);

	return OK;
}
