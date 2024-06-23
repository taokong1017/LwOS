#ifndef __ARM64_SPINLOCK_H__
#define __ARM64_SPINLOCK_H__

#include <types.h>

void arch_spin_lock(uint32_t *lock);
void arch_spin_unlock(uint32_t *lock);
int32_t arch_spin_try_lock(uint32_t *lock);

#endif
