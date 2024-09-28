#ifndef __GENERAL_H__
#define __GENERAL_H__

#define ALIGNED(x) __attribute__((aligned(x)))
#define forever() for (;;)
#define BIT(nr) (1UL << nr)
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32UL - 1 - (h))))
#define GENMASK64(h, l)                                                        \
	(((~0ULL) - (1ULL << (l)) + 1) & (~0ULL >> (64UL - 1 - (h))))

#endif
