#ifndef __SEM_H__
#define __SEM_H__

#include <types.h>
#include <task.h>
#include <kernel.h>

#define SEM_INVALID_ID -1
#define SEM_WAIT_FOREVER WAIT_FOREVER
#define SEM_NO_WAIT NO_WAIT

/* sem name definition */
#define SEM_NAME_LEN 32

/* sem error code definition */
#define ERRNO_SEM_NAME_EMPTY ERRNO_OS_ERROR(MOD_ID_SEM, 0x00)
#define ERRNO_SEM_MAX_COUNT_ZERO ERRNO_OS_ERROR(MOD_ID_SEM, 0x01)
#define ERRNO_SEM_COUNT_INVALID ERRNO_OS_ERROR(MOD_ID_SEM, 0x02)
#define ERRNO_SEM_PTR_NULL ERRNO_OS_ERROR(MOD_ID_SEM, 0x03)
#define ERRNO_SEM_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_SEM, 0x04)
#define ERRNO_SEM_ID_INVALID ERRNO_OS_ERROR(MOD_ID_SEM, 0x05)
#define ERRNO_SEM_TIMEOUT ERRNO_OS_ERROR(MOD_ID_SEM, 0x06)
#define ERRNO_SEM_COUNT_FULL ERRNO_OS_ERROR(MOD_ID_SEM, 0x07)
#define ERRNO_SEM_IS_BUSY ERRNO_OS_ERROR(MOD_ID_SEM, 0x08)

typedef long sem_id_t;

struct ksem {
	sem_id_t id;			 /* semphore id */
	char name[SEM_NAME_LEN]; /* semphore name */

	uint32_t cur_count;			  /* current count */
	uint32_t max_count;			  /* maximum count */
	struct wait_queue wait_queue; /* pending queue */
};

errno_t ksem_create(const char *name, uint32_t count, uint32_t max_count,
					sem_id_t *id);
errno_t ksem_take(sem_id_t id, uint64_t timeout);
errno_t ksem_give(sem_id_t id);
errno_t ksem_irq_give(sem_id_t id);
errno_t ksem_destroy(sem_id_t id);

#endif
