#ifndef __ARM64_CACHE_H__
#define __ARM64_CACHE_H__

#include <types.h>

/*
 *	All functions below apply to the interval [start, end)
 *		- start  - virtual start address (inclusive)
 *		- end    - virtual end address (exclusive)
 *
 *	icache_inval_pou(start, end)
 *
 *		Invalidate I-cache region to the Point of Unification.
 *
 *	dcache_clean_inval_poc(start, end)
 *
 *		Clean and invalidate D-cache region to the Point of Coherency.
 *
 *	dcache_inval_poc(start, end)
 *
 *		Invalidate D-cache region to the Point of Coherency.
 *
 *	dcache_clean_poc(start, end)
 *
 *		Clean D-cache region to the Point of Coherency.
 *
 *	dcache_clean_pop(start, end)
 *
 *		Clean D-cache region to the Point of Persistence.
 */
void icache_inval_pou(uint64_t start, uint64_t end);
void dcache_clean_inval_poc(uint64_t start, uint64_t end);
void dcache_inval_poc(uint64_t start, uint64_t end);
void dcache_clean_poc(uint64_t start, uint64_t end);
void dcache_clean_pop(uint64_t start, uint64_t end);

#endif
