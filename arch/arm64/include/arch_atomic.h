#ifndef __ARM64_ATOMIC_H__
#define __ARM64_ATOMIC_H__

#include <types.h>
#include <compiler.h>

typedef int32_t atomic_t;

bool atomic_cas(atomic_t *target, atomic_t old_value, atomic_t new_value);
atomic_t atomic_add(atomic_t *target, atomic_t value);
atomic_t atomic_sub(atomic_t *target, atomic_t value);
atomic_t atomic_inc(atomic_t *target);
atomic_t atomic_dec(atomic_t *target);
atomic_t atomic_get(const atomic_t *target);
atomic_t atomic_set(atomic_t *target, atomic_t value);
atomic_t atomic_clear(atomic_t *target);
atomic_t atomic_or(atomic_t *target, atomic_t value);
atomic_t atomic_xor(atomic_t *target, atomic_t value);
atomic_t atomic_and(atomic_t *target, atomic_t value);
atomic_t atomic_nand(atomic_t *target, atomic_t value);

#endif
