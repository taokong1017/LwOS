#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include <types.h>
#include <errno.h>
#include <menuconfig.h>
#include <arch_atomic.h>

#define ERRNO_SPINLOCK_TRY_LOCK_FAILED ERRNO_OS_WARN(MOD_ID_SPINLOCK, 0x00)

struct spinlock_key {
	uint32_t key;
};

struct spinlock {
#ifdef CONFIG_SMP
	atomic_t owner;
	atomic_t tail;
#else
	atomic_t locked;
#endif
};

struct spinlock_key spin_lock(struct spinlock *lock);
int32_t spin_trylock(struct spinlock *lock, struct spinlock_key *key);
void spin_unlock(struct spinlock *lock, struct spinlock_key key);
bool spin_is_locked(struct spinlock *lock);
void spin_release(struct spinlock *lock);

#endif
