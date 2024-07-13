#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <types.h>
#include <errno.h>

#define ERRNO_MEM_NULL_PTR ERRNO_OS_ERROR(MOD_ID_MEM, 0x00)
#define ERRNO_MEM_NO_MEMORY ERRNO_OS_ERROR(MOD_ID_MEM, 0x01)
#define ERRNO_MEM_ADDR_INVALID ERRNO_OS_ERROR(MOD_ID_MEM, 0x02)
#define ERRNO_MEM_NO_ALIGN ERRNO_OS_ERROR(MOD_ID_MEM, 0x02)

errno_t mem_init(void *mem, uint32_t size);
void *mem_malloc(uint32_t size);
void mem_free(void *ptr);
void *mem_realloc(void *ptr, uint32_t size);
void *mem_calloc(uint32_t nelem, uint32_t elem_size);

#endif