#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>

int sem_post(sem_t *sem) {
	errno_t errno = OK;
	mutex_id_t mutex_id = mutex_id(sem_locker);

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) != sem->id) {
		user_mutex_give(mutex_id);
		return -1;
	}

	if (sem->cur_count >= sem->max_count) {
		user_mutex_give(mutex_id);
		return -1;
	}

	errno = user_sem_give(sem2id(sem));
	if (errno != OK) {
		user_mutex_give(mutex_id);
		return -1;
	}

	user_mutex_give(mutex_id);

	return 0;
}
