#ifndef __ARM64_COMPILER_H
#define __ARM64_COMPILER_H

#include <types.h>

typedef int8_t __attribute__((__may_alias__)) __u8_alias_t;
typedef uint16_t __attribute__((__may_alias__)) __u16_alias_t;
typedef uint32_t __attribute__((__may_alias__)) __u32_alias_t;
typedef uint64_t __attribute__((__may_alias__)) __u64_alias_t;

#define wfe() __asm__ __volatile__("wfe")
#define wfi() __asm__ __volatile__("wfi")
#define sev() __asm__ __volatile__("sev" : : : "memory")
#define barrier() __asm__ __volatile__("" : : : "memory")
#define isb(option) __asm__ __volatile__("isb " #option : : : "memory")
#define dsb(option) __asm__ __volatile__("dsb " #option : : : "memory")
#define dmb(option) __asm__ __volatile__("dmb " #option : : : "memory")
#define code_unreachable() __builtin_unreachable()

static inline void __read_once_size(const volatile void *p, void *res,
									int size) {
	switch (size) {
	case 1:
		*(__u8_alias_t *)res = *(volatile __u8_alias_t *)p;
		break;
	case 2:
		*(__u16_alias_t *)res = *(volatile __u16_alias_t *)p;
		break;
	case 4:
		*(__u32_alias_t *)res = *(volatile __u32_alias_t *)p;
		break;
	case 8:
		*(__u64_alias_t *)res = *(volatile __u64_alias_t *)p;
		break;
	default:
		barrier();
		__builtin_memcpy((void *)res, (const void *)p, size);
		barrier();
	}
}

static inline void __write_once_size(volatile void *p, void *res, int size) {
	switch (size) {
	case 1:
		*(volatile __u8_alias_t *)p = *(__u8_alias_t *)res;
		break;
	case 2:
		*(volatile __u16_alias_t *)p = *(__u16_alias_t *)res;
		break;
	case 4:
		*(volatile __u32_alias_t *)p = *(__u32_alias_t *)res;
		break;
	case 8:
		*(volatile __u64_alias_t *)p = *(__u64_alias_t *)res;
		break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define read_once(x)                                                           \
	({                                                                         \
		union {                                                                \
			typeof(x) __val;                                                   \
			char __c[1];                                                       \
		} __u = {.__c = {0}};                                                  \
		__read_once_size(&(x), __u.__c, sizeof(x));                            \
		__u.__val;                                                             \
	})

#define write_once(x, val)                                                     \
	({                                                                         \
		union {                                                                \
			typeof(x) __val;                                                   \
			char __c[1];                                                       \
		} __u = {.__val = (val)};                                              \
		__write_once_size(&(x), __u.__c, sizeof(x));                           \
		__u.__val;                                                             \
	})

#define ALIGNED(x) __attribute__((aligned(x)))
#define forever() for (;;)

#endif
