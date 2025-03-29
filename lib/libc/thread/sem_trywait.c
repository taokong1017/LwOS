#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>

int sem_trywait(sem_t *sem) {
	mutex_id_t mutex_id = mutex_id(sem_locker);
	errno_t errno = OK;

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) != sem->id) {
		user_mutex_give(mutex_id);
		return -1;
	}

	errno = user_sem_take(sem2id(sem), 0);
	if (errno != OK) {
		user_mutex_give(mutex_id);
		return -1;
	}

	user_mutex_give(mutex_id);
	return 0;
}
