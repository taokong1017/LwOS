#include <stdlib.h>
#include <string.h>
#include <user_heap.h>
#include <user_mem_domain.h>

#define HEAP_ALIGN_SIZE 16

extern char __user_heap_start[];
extern char __user_heap_end[];
global_data_section struct user_heap malloc_heap = {0};

void *malloc(size_t size) {
	if (!malloc_heap.heap) {
		return NULL;
	}

	return user_heap_aligned_alloc(&malloc_heap, HEAP_ALIGN_SIZE, size);
}

void *aligned_alloc(size_t alignment, size_t size) {
	if (!malloc_heap.heap) {
		return NULL;
	}

	return user_heap_aligned_alloc(&malloc_heap, alignment, size);
}

static inline bool size_mul_overflow(size_t a, size_t b, size_t *result) {
	size_t c = a * b;

	*result = c;

	return a != 0 && (c / a) != b;
}

void *calloc(size_t nmemb, size_t size) {
	void *ret = NULL;

	if (!malloc_heap.heap) {
		return NULL;
	}

	if (size_mul_overflow(nmemb, size, &size)) {
		return NULL;
	}

	ret = malloc(size);

	if (ret != NULL) {
		memset(ret, 0, size);
	}

	return ret;
}

void *realloc(void *ptr, size_t requested_size) {
	if (!malloc_heap.heap) {
		return NULL;
	}

	return user_heap_aligned_realloc(&malloc_heap, ptr, HEAP_ALIGN_SIZE,
									 requested_size);
}

void free(void *ptr) {
	if (!malloc_heap.heap) {
		return;
	}

	user_heap_free(&malloc_heap, ptr);
}

#ifdef CONFIG_USER_SPACE
void uheap_init() {
	user_heap_init(&malloc_heap, __user_heap_start,
				   __user_heap_end - __user_heap_start);
}
#endif
