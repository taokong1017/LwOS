#ifndef __ARM64_ATOMIC_H__
#define __ARM64_ATOMIC_H__

#include <types.h>
#include <compiler.h>

typedef struct {
	int counter;
} atomic_t;

#define __stringify(x...) #x

#define ATOMIC_OP(op, asm_op, constraint)                                      \
	static inline void arch_atomic_##op(int i, atomic_t *v) {                  \
		unsigned long tmp;                                                     \
		atomic_t result;                                                       \
		asm volatile("// arch_atomic_" #op "\n"                                \
					 "prfm pstl1strm, %2\n"                                    \
					 "1: ldxr %w0, %2\n"                                       \
					 "" #asm_op " %w0, %w0, %w3\n"                             \
					 "stxr %w1, %w0, %2\n"                                     \
					 "cbnz %w1, 1b\n"                                          \
					 : "=&r"(result.counter), "=&r"(tmp), "+Q"(v->counter)     \
					 : __stringify(constraint) "r"(i));                        \
	}

#define ATOMIC_OP_RETURN(name, mb, acq, rel, cl, op, asm_op, constraint)       \
	static inline atomic_t arch_atomic_##op##_return##name(int i,              \
														   atomic_t *v) {      \
		unsigned long tmp;                                                     \
		atomic_t result;                                                       \
		asm volatile("// arch_atomic_" #op "_return" #name "\n"                \
					 "prfm pstl1strm, %2\n"                                    \
					 "1: ld" #acq "xr %w0, %2\n"                               \
					 "" #asm_op " %w0, %w0, %w3\n"                             \
					 "st" #rel "xr %w1, %w0, %2\n"                             \
					 "cbnz %w1, 1b\n"                                          \
					 "" #mb                                                    \
					 : "=&r"(result.counter), "=&r"(tmp), "+Q"(v->counter)     \
					 : __stringify(constraint) "r"(i)                          \
					 : cl);                                                    \
		return result;                                                         \
	}

#define ATOMIC_FETCH_OP(name, mb, acq, rel, cl, op, asm_op, constraint)        \
	static inline atomic_t arch_atomic_fetch_##op##name(int i, atomic_t *v) {  \
		unsigned long tmp;                                                     \
		atomic_t val, result;                                                  \
		asm volatile("// arch_atomic_fetch_" #op #name "\n"                    \
					 "prfm pstl1strm, %3\n"                                    \
					 "1: ld" #acq "xr %w0, %3\n"                               \
					 "" #asm_op " %w1, %w0, %w4\n"                             \
					 "st" #rel "xr %w2, %w1, %3\n"                             \
					 "cbnz %w2, 1b\n"                                          \
					 " " #mb                                                   \
					 : "=&r"(result.counter), "=&r"(val.counter), "=&r"(tmp),  \
					   "+Q"(v->counter)                                        \
					 : __stringify(constraint) "r"(i)                          \
					 : cl);                                                    \
		return result;                                                         \
	}

#define ATOMIC_OPS(...)                                                        \
	ATOMIC_OP(__VA_ARGS__)                                                     \
	ATOMIC_OP_RETURN(, dmb ish, , l, "memory", __VA_ARGS__)                    \
	ATOMIC_OP_RETURN(_relaxed, , , , , __VA_ARGS__)                            \
	ATOMIC_OP_RETURN(_acquire, , a, , "memory", __VA_ARGS__)                   \
	ATOMIC_OP_RETURN(_release, , , l, "memory", __VA_ARGS__)                   \
	ATOMIC_FETCH_OP(, dmb ish, , l, "memory", __VA_ARGS__)                     \
	ATOMIC_FETCH_OP(_relaxed, , , , , __VA_ARGS__)                             \
	ATOMIC_FETCH_OP(_acquire, , a, , "memory", __VA_ARGS__)                    \
	ATOMIC_FETCH_OP(_release, , , l, "memory", __VA_ARGS__)

ATOMIC_OPS(add, add, I)
ATOMIC_OPS(sub, sub, J)

#undef ATOMIC_OPS
#define ATOMIC_OPS(...)                                                        \
	ATOMIC_OP(__VA_ARGS__)                                                     \
	ATOMIC_FETCH_OP(, dmb ish, , l, "memory", __VA_ARGS__)                     \
	ATOMIC_FETCH_OP(_relaxed, , , , , __VA_ARGS__)                             \
	ATOMIC_FETCH_OP(_acquire, , a, , "memory", __VA_ARGS__)                    \
	ATOMIC_FETCH_OP(_release, , , l, "memory", __VA_ARGS__)

ATOMIC_OPS(and, and, K)
ATOMIC_OPS(or, orr, K)
ATOMIC_OPS(xor, eor, K)
ATOMIC_OPS(andnot, bic, )

#define arch_atomic_read(v) read_once((v)->counter)
#define arch_atomic_set(v, i) write_once(((v)->counter), (i))

#define ATOMIC_CAS(name, mb, acq, rel, cl)                                     \
	static inline bool arch_atomic_cas##name(atomic_t *v, int old_value,       \
											 int new_value) {                  \
		uint32_t tmp;                                                          \
		atomic_t result;                                                       \
		asm volatile("// arch_atomic_cas\n"                                    \
					 "prfm pstl1strm, %2\n"                                    \
					 "1: ld" #acq "xr %w0, %2\n"                               \
					 "cmp %w0, %w3\n"                                          \
					 "bne 2f\n"                                                \
					 "st" #rel "xr %w1, %w4, %2\n"                             \
					 "cbnz %w1, 1b\n"                                          \
					 "2:\n"                                                    \
					 "" #mb                                                    \
					 : "=&r"(result.counter), "=&r"(tmp), "+Q"(v->counter)     \
					 : "r"(old_value), "r"(new_value)                          \
					 : cl);                                                    \
		return result.counter == old_value;                                    \
	}

ATOMIC_CAS(, dmb ish, , l, "memory")
ATOMIC_CAS(_relaxed, _relaxed, , , )
ATOMIC_CAS(_acquire, _acquire, , a, "memory")
ATOMIC_CAS(_release, _release, , l, "memory")

#endif
