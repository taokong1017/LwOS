#include <spin_lock.h>
#include <task.h>
#include <arch_spinlock.h>
#include <irq.h>
#include <menuconfig.h>

#ifdef CONFIG_SMP
void spin_lock(struct spinlock *lock) {
	task_lock();
	arch_spin_lock(&lock->rawlock);

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

	return;
}

void spin_lock_save(struct spinlock *lock, uint32_t *key) {
	spin_lock(lock);
	*key = arch_irq_save();

	return;
}

void spin_lock_restore(struct spinlock *lock, uint32_t key) {
	spin_lock(lock);
	arch_irq_restore(key);

	return;
}

void spin_lock_init(struct spinlock *lock)
{
	lock->rawlock = 0;
}

bool spin_lock_is_locked(struct spinlock *lock)
{
	return lock->rawlock != 0;
}

#else

void spin_lock(struct spinlock *lock) {
	(void)lock;
}

int32_t spin_trylock(struct spinlock *lock) {
	(void)lock;

	return true;
}

void spin_unlock(struct spinlock *lock) {
	(void)lock;
}

void spin_lock_save(struct spinlock *lock, uint32_t *key) {
	(void)lock;
	*key = arch_irq_save();

	return;
}

void spin_lock_restore(struct spinlock *lock, uint32_t key) {
	(void)lock;
	arch_irq_restore(key);
}

void spin_lock_init(struct spinlock *lock)
{
	(void)lock;
}

bool spin_lock_is_locked(struct spinlock *lock)
{
	return true;
}

#endif
