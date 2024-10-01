#ifndef __GENERAL_H__
#define __GENERAL_H__

#define forever() for (;;)
#define POWER_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a) POWER_MASK(x, (__typeof__(x))(a)-1)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED(x) __attribute__((aligned(x)))
#define PACKED __attribute__((__packed__))
#define BIT(nr) (1UL << nr)
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32UL - 1 - (h))))
#define GENMASK64(h, l)                                                        \
	(((~0ULL) - (1ULL << (l)) + 1) & (~0ULL >> (64UL - 1 - (h))))

#endif
