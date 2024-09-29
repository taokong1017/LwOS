#ifndef __GENERAL_H__
#define __GENERAL_H__

#define forever() for (;;)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED(x) __attribute__((aligned(x)))
#define BIT(nr) (1UL << nr)
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32UL - 1 - (h))))
#define GENMASK64(h, l)                                                        \
	(((~0ULL) - (1ULL << (l)) + 1) & (~0ULL >> (64UL - 1 - (h))))

#endif
