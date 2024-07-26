#include <spin_lock.h>
#include <task.h>
#include <arch_spinlock.h>
#include <irq.h>
#include <menuconfig.h>
#include <task_sched.h>
#include <log.h>

#define SPIN_LOCK_TAG "SPIN_LOCK"
#define default_str_fill(str) ((str == NULL) ? "unkown" : str)

void spin_lock(struct spinlock *lock) {
	task_lock();
	arch_spin_lock(&lock->rawlock);
	lock->owner = current_task_get();

	return;
}

int32_t spin_trylock(struct spinlock *lock) {
	int32_t ret = 0;

	task_lock();
	ret = arch_spin_try_lock(&lock->rawlock);
	if (ret != OK) {
		task_unlock();
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}

	return OK;
}

void spin_unlock(struct spinlock *lock) {
	arch_spin_unlock(&lock->rawlock);
	task_unlock();
	lock->owner = NULL;

	return;
}

void spin_lock_save(struct spinlock *lock, uint32_t *key) {
	*key = arch_irq_save();
	spin_lock(lock);

	return;
}

void spin_lock_restore(struct spinlock *lock, uint32_t key) {
	spin_unlock(lock);
	arch_irq_restore(key);

	return;
}

void spin_lock_init(struct spinlock *lock) { lock->rawlock = 0; }

bool spin_lock_is_locked(struct spinlock *lock) { return lock->rawlock != 0; }

void spin_lock_dump(struct spinlock *lock) {
	if (!lock) {
		return;
	}

	printf("SPIN_LOCK DUMP:\nspin lock: %s, owner: %s, raw lock: %u\n",
		   default_str_fill(lock->name),
		   lock->owner ? default_str_fill(lock->owner->name)
					   : default_str_fill(NULL),
		   lock->rawlock);

	return;
}
