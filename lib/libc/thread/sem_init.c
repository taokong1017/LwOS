#include <semaphore.h>
#include <mutex.h>
#include <user_space.h>
#include <stdio.h>
#include <menuconfig.h>

mutex_define(sem_locker, sem_locker);

int sem_init(sem_t *sem, int pshared, unsigned int value) {
	mutex_id_t mutex_id = mutex_id(sem_locker);
	static int libc_sem_num = 0;
	(void)pshared;

	if (value > CONFIG_LIBC_SEM_VALUE_MAX) {
		return -1;
	}

	user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
	if (!sem || sem2id(sem) == sem->id) {
		user_mutex_give(mutex_id);
		return -1;
	}

	sem->id = sem2id(sem);
	sprintf(sem->name, "libc_sem%d", ++libc_sem_num);
	sem->name[SEM_NAME_LEN - 1] = '\0';
	sem->cur_count = value;
	sem->max_count = CONFIG_LIBC_SEM_VALUE_MAX;
	INIT_LIST_HEAD(&sem->wait_queue.wait_list);
	user_mutex_give(mutex_id);

	return 0;
}
