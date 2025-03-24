#include <string.h>
#include <sem.h>
#include <mem_mgr.h>
#include <log.h>
#include <task_sched.h>

#define SEM_TAG "SEM"
#define sem2id(sem) ((sem_id_t)sem)
#define id2sem(id) ((struct sem *)id)

errno_t sem_create(const char *name, uint32_t count, uint32_t max_count,
				   sem_id_t *id) {
	struct sem *sem = NULL;

	if (!name) {
		log_err(SEM_TAG, "the sem name is empty\n");
		return ERRNO_SEM_NAME_EMPTY;
	}

	if (max_count == 0) {
		log_err(SEM_TAG, "the maximum count is zero\n");
		return ERRNO_SEM_MAX_COUNT_ZERO;
	}

	if (count > max_count) {
		log_err(SEM_TAG, "the count %u is larger than maximum count %u\n",
				count, max_count);
		return ERRNO_SEM_COUNT_INVALID;
	}

	if (!id) {
		log_err(SEM_TAG, "the sem id is NULL\n");
		return ERRNO_SEM_PTR_NULL;
	}

	sem = (struct sem *)kmalloc(sizeof(struct sem));
	if (!sem) {
		log_err(SEM_TAG, "the memory is not enough\n");
		return ERRNO_SEM_NO_MEMORY;
	}

	sem->id = sem2id(sem);
	strncpy(sem->name, name, SEM_NAME_LEN);
	sem->name[SEM_NAME_LEN - 1] = '\0';
	sem->cur_count = count;
	sem->max_count = max_count;
	INIT_LIST_HEAD(&sem->wait_queue.wait_list);
	*id = sem->id;

	return OK;
}

errno_t sem_take(sem_id_t id, uint64_t timeout) {
	struct sem *sem = id2sem(id);
	uint32_t key = 0;
	errno_t ret = OK;

	if (!sem || id != sem->id) {
		log_err(SEM_TAG, "the sem id %ld is invalid\n", id);
		return ERRNO_SEM_ID_INVALID;
	}

	key = sched_spin_lock();

	if (sem->cur_count > 0) {
		sem->cur_count--;
		sched_spin_unlock(key);
		log_debug(SEM_TAG, "the sem %s is taken without waiting\n", sem->name);
		return ret;
	}

	ret = task_wait_locked(&sem->wait_queue, timeout);
	if (ret != OK) {
		ret = ERRNO_SEM_TIMEOUT;
		log_err(SEM_TAG, "the sem %s is taken failed\n", sem->name);
	} else {
		log_debug(SEM_TAG, "the sem %s is taken\n", sem->name);
	}

	sched_spin_unlock(key);

	return ret;
}

errno_t sem_give(sem_id_t id) {
	struct sem *sem = id2sem(id);
	uint32_t key = 0;

	if (!sem || id != sem->id) {
		log_err(SEM_TAG, "the sem id %ld is invalid\n", id);
		return ERRNO_SEM_ID_INVALID;
	}

	key = sched_spin_lock();

	if (list_empty(&sem->wait_queue.wait_list)) {
		if (sem->cur_count < sem->max_count) {
			sem->cur_count++;
			log_debug(SEM_TAG, "the sem %s is given\n", sem->name);
		} else {
			sched_spin_unlock(key);
			log_debug(SEM_TAG, "the sem %s is full\n", sem->name);
			return ERRNO_SEM_COUNT_FULL;
		}
	} else {
		task_wakeup_locked(&sem->wait_queue);
		log_debug(SEM_TAG, "the sem %s wakeup an waiting task\n", sem->name);
	}

	sched_spin_unlock(key);

	return OK;
}

errno_t sem_irq_give(sem_id_t id) {
	struct sem *sem = id2sem(id);

	if (!sem || id != sem->id) {
		log_err(SEM_TAG, "the sem id %ld is invalid\n", id);
		return ERRNO_SEM_ID_INVALID;
	}

	if (list_empty(&sem->wait_queue.wait_list)) {
		if (sem->cur_count < sem->max_count) {
			sem->cur_count++;
			log_debug(SEM_TAG, "the sem %s is given\n", sem->name);
		} else {
			log_debug(SEM_TAG, "the sem %s is full\n", sem->name);
			return ERRNO_SEM_COUNT_FULL;
		}
	} else {
		task_wakeup_locked(&sem->wait_queue);
		log_debug(SEM_TAG, "the sem %s wakeup an waiting task\n", sem->name);
	}

	return OK;
}

errno_t sem_destroy(sem_id_t id) {
	struct sem *sem = id2sem(id);

	if (!sem || id != sem->id) {
		log_err(SEM_TAG, "the sem id %ld is invalid\n", id);
		return ERRNO_SEM_ID_INVALID;
	}

	if (!list_empty(&sem->wait_queue.wait_list)) {
		log_err(SEM_TAG, "the sem %s is busy\n", sem->name);
		return ERRNO_SEM_IS_BUSY;
	}

	sem->id = SEM_INVALID_ID;
	kfree(sem);

	return OK;
}
