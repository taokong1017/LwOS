#ifndef __USER_HEAP_H__
#define __USER_HEAP_H__

#include <types.h>
#include <heap.h>
#include <general.h>

struct user_heap {
	struct heap *heap;
	void *init_mem;
	size_t init_bytes;
} ALIGNED(16);

void user_heap_init(struct user_heap *heap, void *mem, size_t bytes);
void *user_heap_alloc(struct user_heap *heap, size_t bytes);
void *user_heap_aligned_alloc(struct user_heap *heap, size_t align,
							  size_t bytes);
void *user_heap_aligned_realloc(struct user_heap *heap, void *ptr, size_t align,
								size_t bytes);
void user_heap_free(struct user_heap *heap, void *mem);

#endif
