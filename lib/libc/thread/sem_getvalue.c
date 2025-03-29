#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>

int sem_getvalue(sem_t *sem, int *valp) {
	errno_t errno = OK;
	mutex_id_t mutex_id = mutex_id(sem_locker);

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) != sem->id || !valp) {
		user_mutex_give(mutex_id);
		return -1;
	}

	errno = user_sem_value_get(sem->id, (uint32_t *)valp);
	if (errno) {
		user_mutex_give(mutex_id);
		return -1;
	}

	user_mutex_give(mutex_id);

	return 0;
}
