#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__

#include <types.h>
#include <errno.h>
#include <menuconfig.h>
#include <arch_atomic.h>

#define OWNER_NAME_LEN 32
#define STACK_WALK_SIZE 20
#define SPIN_LOCK_DEFINE(locker, locker_name)                                  \
	struct spinlock locker = {.rawlock = 0, .name = locker_name, .level = 0};
#define SPIN_LOCK_DECLARE(locker) extern struct spinlock locker;
#define ERRNO_SPINLOCK_TRY_LOCK_FAILED ERRNO_OS_WARN(MOD_ID_SPINLOCK, 0x00)

struct spinlock {
	uint32_t rawlock;
	const char *name;
	uint32_t level;
	uint32_t cpu_id;
	char owner[OWNER_NAME_LEN];
	virt_addr_t trace[STACK_WALK_SIZE];
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
