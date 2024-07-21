#include <arch_atomic.h>

bool atomic_cas(atomic_t *target, atomic_t old_value, atomic_t new_value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "cmp %w0, %w3\n"
					 "bne 2f\n"
					 "stxr %w1, %w4, %2\n"
					 "cbnz %w1, 1b\n"
					 "2:\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(old_value), "r"(new_value)
					 : "cc", "memory");

	return result == new_value;
}

atomic_t atomic_add(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "add %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "cc", "memory");

	return result;
}

atomic_t atomic_sub(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "sub %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "cc", "memory");

	return result;
}

atomic_t atomic_inc(atomic_t *target) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "add %w0, %w0, #1\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)::"cc",
					   "memory");

	return result;
}

atomic_t atomic_dec(atomic_t *target) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "sub %w0, %w0, #1\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)::"cc",
					   "memory");

	return result;
}

atomic_t atomic_get(atomic_t *target) {
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %1\n"
					 "ldxr %w0, %1\n"
					 : "=&r"(result), "+Q"(*target));

	return result;
}

atomic_t atomic_set(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;

	__asm__ volatile("prfm pstl1strm, %1\n"
					 "ldxr %w0, %1\n"
					 "1: stxr %w0, %w2, %1\n"
					 "cbnz %w0, 1b\n"
					 "dmb sy\n"
					 : "+r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "memory");

	return value;
}

atomic_t atomic_clear(atomic_t *target) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "ldxr %w1, %2\n"
					 "1: stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "+r"(tmp), "+Q"(*target)::"memory");

	return result;
}

atomic_t atomic_or(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "orr %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "memory");

	return result;
}

atomic_t atomic_xor(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "eor %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "memory");

	return result;
}

atomic_t atomic_and(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "and %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "memory");

	return result;
}

atomic_t atomic_nand(atomic_t *target, atomic_t value) {
	atomic_t tmp = 0;
	atomic_t result = 0;

	__asm__ volatile("prfm pstl1strm, %2\n"
					 "1: ldxr %w0, %2\n"
					 "bic %w0, %w0, %w3\n"
					 "stxr %w1, %w0, %2\n"
					 "cbnz %w1, 1b\n"
					 "dmb sy\n"
					 : "=&r"(result), "=&r"(tmp), "+Q"(*target)
					 : "r"(value)
					 : "memory");

	return result;
}
