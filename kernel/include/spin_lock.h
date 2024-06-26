#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include <types.h>
#include <errno.h>
#include <menuconfig.h>
#include <arch_atomic.h>

#define ERRNO_SPINLOCK_TRY_LOCK_FAILED ERRNO_OS_WARN(MOD_ID_SPINLOCK, 0x00)
#define NAME_LEN 32
#define SPIN_LOCK_DEFINE(locker)                                               \
	struct spinlock locker = {.rawlock = 0, .owner = NULL, .name = #locker}

struct spinlock {
	uint32_t rawlock;
	struct task *owner;
	char name[NAME_LEN];
};

void spin_lock(struct spinlock *lock);
int32_t spin_trylock(struct spinlock *lock);
void spin_unlock(struct spinlock *lock);
void spin_lock_init(struct spinlock *lock);
bool spin_lock_is_locked(struct spinlock *lock);
void spin_lock_save(struct spinlock *lock, uint32_t *key);
void spin_lock_restore(struct spinlock *lock, uint32_t key);
void spin_lock_dump(struct spinlock *lock);

#endif
