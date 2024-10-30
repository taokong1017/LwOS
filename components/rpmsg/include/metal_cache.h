#ifndef __METAL_CACHE_H__
#define __METAL_CACHE_H__

#include <types.h>

static inline void metal_cache_flush(void *addr, uint32_t len) {
	if (!addr || !len) {
		return;
	}
}

static inline void metal_cache_invalidate(void *addr, uint32_t len) {
	if (!addr || !len) {
		return;
	}
}

#endif
