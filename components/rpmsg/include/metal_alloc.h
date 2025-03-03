#ifndef __METAL_ALLOC_H__
#define __METAL_ALLOC_H__

#include <types.h>
#include <menuconfig.h>
#ifdef CONFIG_USER_SPACE
#include <stdlib.h>
#else
#include <mem_mgr.h>
#endif

static inline void *metal_allocate_memory(uint32_t size) {
#ifdef CONFIG_USER_SPACE
	return malloc(size);
#else
	return kmalloc(size);
#endif
}

static inline void metal_free_memory(void *ptr) {
#ifdef CONFIG_USER_SPACE
	free(ptr);
#else
	kfree(ptr);
#endif
}

#endif