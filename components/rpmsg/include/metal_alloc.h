#ifndef __METAL_ALLOC_H__
#define __METAL_ALLOC_H__

#include <types.h>
#include <menuconfig.h>
#ifdef CONFIG_KERNEL_USE
#include <mem_mgr.h>
#else
#include <stdlib.h>
#endif

static inline void *metal_allocate_memory(uint32_t size) {
#ifdef CONFIG_KERNEL_USE
	return kmalloc(size);
#else
	return malloc(size);
#endif
}

static inline void metal_free_memory(void *ptr) {
#ifdef CONFIG_KERNEL_USE
	kfree(ptr);
#else
	free(ptr);
#endif
}

#endif