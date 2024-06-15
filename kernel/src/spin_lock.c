#include <spin_lock.h>
#include <irq.h>

struct spinlock_key spin_lock(struct spinlock *lock) {
	struct spinlock_key lock_key = {0};

	lock_key.key = arch_irq_save();
	atomic_t ticket = arch_atomic_add_return(1, &lock->tail);
	while (arch_atomic_read(&ticket) - arch_atomic_read(&lock->owner) > 1)
		;

	return lock_key;
}

int32_t spin_trylock(struct spinlock *lock, struct spinlock_key *key) {
	uint32_t val = arch_irq_save();

	int ticket_val = arch_atomic_read(&lock->owner);
	if (!arch_atomic_cas(&lock->tail, ticket_val, ticket_val + 1)) {
		arch_irq_restore(val);
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}

	key->key = val;
	return OK;
}

void spin_unlock(struct spinlock *lock, struct spinlock_key key) {
	arch_atomic_add(1, &lock->owner);
	arch_irq_restore(key.key);

	return;
}

bool spin_is_locked(struct spinlock *lock) {
	uint32_t ticket_val = arch_atomic_read(&lock->owner);

	return !arch_atomic_cas(&lock->tail, ticket_val, ticket_val);
}

void spin_release(struct spinlock *lock) {
	arch_atomic_add(1, &lock->owner);
}
