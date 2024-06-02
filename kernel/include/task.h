#ifndef __TASK_H__
#define __TASK_H__

#include <list.h>
#include <types.h>
#include <errno.h>

/* task default invalid ID definition */
#define TASK_INVALID_CPUID 0xFFFF
#define TASK_INVALID_ID 0xFFFFFFFF

/* task name definition */
#define TASK_NAME_LEN 32

/* task priority definition */
#define TASK_PRIORITY_HIGHEST 31
#define TASK_PRIORITY_LOWEST 0
#define TASK_PRIORITY_NUM (TASK_PRIORITY_HIGHEST - TASK_PRIORITY_LOWEST + 1)

/* task status definition */
#define TASK_STATUS_STOP 0x0001U
#define TASK_STATUS_READY 0x0002U
#define TASK_STATUS_SUSPEND 0x0004U
#define TASK_STATUS_PEND 0x0008U
#define TASK_STATUS_RUNNING 0x0010U

/* task flag definition */
#define TASK_FLAG_DETACHED 0x0001U
#define TASK_FLAG_SYSTEM 0x0002U

/* task stack size definition */
#define TASK_STACK_SIZE_ALIGN 16U
#define TASK_STACK_ADDR_ALIGN 8U

/* task error code definition */
#define ERRNO_TASK_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_TASK, 0x00)
#define ERRNO_TASK_PTR_NULL ERRNO_OS_ERROR(MOD_ID_TASK, 0x01)
#define ERRNO_TASK_STACK_ALIGN ERRNO_OS_ERROR(MOD_ID_TASK, 0x02)
#define ERRNO_TASK_PRIOR_ERROR ERRNO_OS_ERROR(MOD_ID_TASK, 0x03)
#define ERRNO_TASK_ENTRY_NULL ERRNO_OS_ERROR(MOD_ID_TASK, 0x04)
#define ERRNO_TASK_NAME_EMPTY ERRNO_OS_ERROR(MOD_ID_TASK, 0x05)
#define ERRNO_TASK_STKSZ_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x06)
#define ERRNO_TASK_ID_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x07)

/* task entry definition */
typedef void (*task_entry_t)(void *p1, void *p2, void *p3);
typedef long task_id_t;

/* task structure definition */
struct task {
	task_id_t id;
	char name[TASK_NAME_LEN];
	uint32_t status;
	uint32_t priority;
	void *stack_ptr;
	uint32_t stack_size;
	task_entry_t entry;
	void *args[4];
	struct list_head pend_list;
};

void task_announce();
errno_t task_create(task_id_t *task_id, const char name[TASK_NAME_LEN],
					task_entry_t entry, void *arg0, void *arg1, void *arg2,
					void *arg3, uint32_t stack_size);
errno_t task_start(task_id_t task_id);
errno_t task_stop(task_id_t task_id);
errno_t task_stop_self();
errno_t task_resume(task_id_t task_id);
errno_t task_suspend(task_id_t task_id);
errno_t task_suspend_self();
errno_t task_delay(task_id_t tick);
errno_t task_prority_set(task_id_t task_id, uint32_t prioriy);
errno_t task_prority_get(task_id_t task_id, uint32_t *prioriy);
errno_t task_cpu_affi_set(task_id_t task_id, uint32_t affi_mask);
errno_t task_cpu_affi_get(task_id_t task_id, uint32_t *affi_mask);
void task_lock();
void task_unlock();

#endif