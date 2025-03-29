#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>

#define sem_max_count(sem) (sem->max_count)

int sem_destroy(sem_t *sem) {
	int32_t count = 0;
	mutex_id_t mutex_id = mutex_id(sem_locker);

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) != sem->id) {
		user_mutex_give(mutex_id);
		return -1;
	}

	if (sem_getvalue(sem, &count) || (count != sem_max_count(sem))) {
		return -1;
	}

	sem->id = SEM_INVALID_ID;
	user_mutex_give(mutex_id);

	return 0;
}
