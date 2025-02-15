#ifndef __MEMORY_MGR_H__
#define __MEMORY_MGR_H__

#include <types.h>
#include <errno.h>

#define ERRNO_MEM_NULL_PTR ERRNO_OS_ERROR(MOD_ID_MEM, 0x00)
#define ERRNO_MEM_NO_MEMORY ERRNO_OS_ERROR(MOD_ID_MEM, 0x01)
#define ERRNO_MEM_ADDR_INVALID ERRNO_OS_ERROR(MOD_ID_MEM, 0x02)
#define ERRNO_MEM_NO_ALIGN ERRNO_OS_ERROR(MOD_ID_MEM, 0x02)

void kheap_init();
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint32_t size);
void *kcalloc(uint32_t nelem, uint32_t elem_size);

#endif