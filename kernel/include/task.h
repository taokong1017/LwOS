#ifndef __TASK_H__
#define __TASK_H__

#include <list.h>
#include <types.h>

/* task default invalid ID definition */
#define TASK_INVALID_CPUID 0xFFFF
#define TASK_INVALID_ID 0xFFFFFFFF

/* task priority definition */
#define TASK_PRIORITY_HIGHEST 31
#define TASK_PRIORITY_LOWEST 0
#define TASK_PRIORITY_NUM (TASK_PRIORITY_HIGHEST - TASK_PRIORITY_LOWEST + 1)

/* task status definition */
#define TASK_STATUS_UNUSED 0x0001U
#define TASK_STATUS_SUSPEND 0x0002U
#define TASK_STATUS_READY 0x0004U
#define TASK_STATUS_PEND 0x0008U
#define TASK_STATUS_RUNNING 0x0010U
#define TASK_STATUS_DELAY 0x0020U
#define TASK_STATUS_TIMEOUT 0x0040U
#define TASK_STATUS_PEND_TIME 0x0080U

/* task flag definition */
#define TASK_FLAG_DETACHED 0x0001U
#define TASK_FLAG_SYSTEM 0x0002U

/* task stack size definition */
#define TASK_STACK_SIZE_ALIGN 16U
#define TASK_STACK_ADDR_ALIGN 8U

/* task entry definition */
typedef void (*task_entry_t)(void *p1, void *p2, void *p3);

/* task structure definition */
struct task {
	char name[32];
	uint32_t id;
	uint32_t status;
	uint32_t flags;
	uint32_t priority;
	void *stack_ptr;
	uint32_t stack_size;
	task_entry_t entry;
	void *args[4];
	struct list_head pend_list;
#ifdef CONFIF_SMP
	uint16_t curr_cpu;
	uint16_t last_cpu;
	uint16_t timer_cpu;
	uint16_t cpu_aff_Mask;
#endif
};

#endif