#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <types.h>

void *mem_alloc(uint32_t size);
void *mem_alloc_align(uint32_t size, uint32_t align);
void mem_free(void *ptr);

#endif