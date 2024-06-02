#include <memory.h>

char heap[0x10000000] = {0};
uint32_t used = 0;

#define ALIGN(addr, align) ((addr + align - 1) & ~(align - 1))

void *mem_alloc(uint32_t size) {
	char *start = heap + used;
	char *end = start + size;
	used += end - start;
	return start;
}

void *mem_alloc_align(uint32_t size, uint32_t align) {
	char *start = heap + used;
	char *end = (char *)ALIGN((unsigned long)start + size, align);
	used += end - start;
	return start;
}

void mem_free(void *ptr) {}
