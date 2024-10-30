#ifndef __METAL_ALLOC_H__
#define __METAL_ALLOC_H__

#include <types.h>
#include <mem_mgr.h>

static inline void *metal_allocate_memory(uint32_t size) {
	return mem_malloc(size);
}

static inline void metal_free_memory(void *ptr) { mem_free(ptr); }

#endif