#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
	mutex_id_t mutex_id = mutex_id(sem_locker);
	errno_t errno = OK;

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) != sem->id || !abs_timeout) {
		user_mutex_give(mutex_id);
		return -1;
	}

	// TO DO
	errno = user_sem_take(sem2id(sem), 0);
	if (errno != OK) {
		user_mutex_give(mutex_id);
		return -1;
	}

	user_mutex_give(mutex_id);
	return 0;
}
