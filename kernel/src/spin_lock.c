#include <spin_lock.h>
#include <irq.h>

struct spinlock_key spin_lock(struct spinlock *lock) {
	struct spinlock_key lock_key = {0};

	lock_key.key = arch_irq_save();
#ifdef CONFIG_SMP
	atomic_t ticket = arch_atomic_add_return(1, &lock->tail);
	while (arch_atomic_read(&lock->owner) != arch_atomic_read(&ticket)) {
		// TO DO
	}
#else
	while (!arch_atomic_cas(&lock->locked, 0, 1)) {
		// TO DO
	}
#endif

	return lock_key;
}

int32_t spin_trylock(struct spinlock *lock, struct spinlock_key *key) {
	uint32_t val = arch_irq_save();

#ifdef CONFIG_SMP
	int ticket_val = arch_atomic_read(&lock->owner);
	if (!arch_atomic_cas(&lock->tail, ticket_val, ticket_val + 1)) {
		arch_irq_restore(val);
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}
#else
	if (!arch_atomic_cas(&lock->locked, 0, 1)) {
		arch_irq_restore(val);
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}
#endif

	key->key = val;
	return OK;
}

void spin_unlock(struct spinlock *lock, struct spinlock_key key) {
#ifdef CONFIG_SMP
	arch_atomic_add(1, &lock->owner);
#else
	arch_atomic_set(&lock->locked, 0);
#endif

	arch_irq_restore(key.key);
	return;
}

bool spin_is_locked(struct spinlock *lock) {
#ifdef CONFIG_SMP
	uint32_t ticket_val = arch_atomic_read(&lock->owner);
	return !arch_atomic_cas(&lock->tail, ticket_val, ticket_val);
#else
	return arch_atomic_read(&lock->locked);
#endif
}

void spin_release(struct spinlock *lock) {
#ifdef CONFIG_SMP
	arch_atomic_add(1, &lock->owner);
#else
	arch_atomic_set(&lock->locked, 0);
#endif

	return;
}
