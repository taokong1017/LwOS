#ifndef __TASK_H__
#define __TASK_H__

#include <list.h>
#include <types.h>
#include <errno.h>
#include <timeout.h>
#include <arch_task.h>
#include <stack_trace.h>

#define ID_TO_TASK(task_id) ((struct task *)task_id)

/* task default invalid ID definition */
#define TASK_INVALID_CPUID 0xFFFF
#define TASK_INVALID_ID 0

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
#define TASK_FLAG_SYSTEM 0x0001U
#define TASK_FLAG_KERNEL 0x0002U
#define TASK_FLAG_USER 0x0004U
#define TASK_FLAG_DETACHED 0x0008U
#define TASK_DEFAULT_FLAG (TASK_FLAG_SYSTEM | TASK_FLAG_KERNEL)
#define TASK_FLAG_MASK                                                         \
	(TASK_FLAG_SYSTEM | TASK_FLAG_KERNEL | TASK_FLAG_USER | TASK_FLAG_DETACHED)

/* task stack size definition */
#define TASK_STACK_SIZE_ALIGN 16U
#define TASK_STACK_ADDR_ALIGN 8U
#define TASK_STACK_DEFAULT_SIZE 8192U

/* task error code definition */
#define ERRNO_TASK_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_TASK, 0x00)
#define ERRNO_TASK_PTR_NULL ERRNO_OS_ERROR(MOD_ID_TASK, 0x01)
#define ERRNO_TASK_STACK_ALIGN ERRNO_OS_ERROR(MOD_ID_TASK, 0x02)
#define ERRNO_TASK_PRIOR_ERROR ERRNO_OS_ERROR(MOD_ID_TASK, 0x03)
#define ERRNO_TASK_ENTRY_NULL ERRNO_OS_ERROR(MOD_ID_TASK, 0x04)
#define ERRNO_TASK_NAME_EMPTY ERRNO_OS_ERROR(MOD_ID_TASK, 0x05)
#define ERRNO_TASK_STKSZ_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x06)
#define ERRNO_TASK_ID_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x07)
#define ERRNO_TASK_STATUS_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x08)
#define ERRNO_TASK_PRIORITY_EMPTY ERRNO_OS_ERROR(MOD_ID_TASK, 0x09)
#define ERRNO_TASK_FLAG_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x0a)
#define ERRNO_TASK_IN_IRQ_STATUS ERRNO_OS_ERROR(MOD_ID_TASK, 0x0b)
#define ERRNO_TASK_OPERATE_INVALID ERRNO_OS_ERROR(MOD_ID_TASK, 0x0c)
#define ERRNO_TASK_IS_LOCKED ERRNO_OS_ERROR(MOD_ID_TASK, 0x0c)

/* task cpu affinity */
#define TASK_CPU_DEFAULT_AFFI 0x00000001U
#define TASK_CPU_AFFI_MASK ((1U < CONFIG_CPUS_MAX_NUM) - 1)

#define task_saved_fp(task) (task->task_context.callee_context.x29)
#define task_saved_lr(task) (task->task_context.callee_context.x30)
#define task_stack_info(task)                                                  \
	((struct stack_info){                                                      \
		((phys_addr_t)((task)->stack_ptr) - (phys_addr_t)(task)->stack_size),  \
		(phys_addr_t)((task)->stack_ptr)})

/* task entry definition */
typedef void (*task_entry_func)(void *arg0, void *arg1, void *arg2, void *arg3);

struct wait_queue {
	struct list_head wq;
};

/* task structure definition */
struct task {
	task_id_t id;
	char name[TASK_NAME_LEN];
	uint32_t status;
	uint32_t priority;
	uint32_t cpu_affi;
	uint32_t flag;
	void *stack_ptr;
	uint32_t stack_size;
	uint64_t lock_cnt;
	uint32_t cpu_id;
	task_entry_func entry;
	void *args[4];
	struct list_head node;
	struct wait_queue *wait_queue_ptr;
	struct wait_queue halt_queue;
	struct timeout timeout;
	struct arch_task_context task_context;
};

errno_t task_create(task_id_t *task_id, const char name[TASK_NAME_LEN],
					task_entry_func entry, void *arg0, void *arg1, void *arg2,
					void *arg3, uint32_t stack_size, uint32_t flag);
errno_t task_start(task_id_t task_id);
errno_t task_stop(task_id_t task_id);
errno_t task_stop_self();
errno_t task_resume(task_id_t task_id);
errno_t task_suspend(task_id_t task_id);
errno_t task_suspend_self();
errno_t task_delay(uint64_t ticks);
errno_t task_prority_set(task_id_t task_id, uint32_t prioriy);
errno_t task_prority_get(task_id_t task_id, uint32_t *prioriy);
errno_t task_cpu_affi_set(task_id_t task_id, uint32_t cpu_affi);
errno_t task_cpu_affi_get(task_id_t task_id, uint32_t *cpu_affi);
task_id_t task_self_id();
void task_lock();
void task_unlock();

#endif