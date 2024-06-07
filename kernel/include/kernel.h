#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <types.h>
#include <list.h>
#include <menuconfig.h>
#include <task.h>

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define PRIQ_BITMAP_SIZE (DIV_ROUND_UP(TASK_PRIORITY_NUM, 8 * sizeof(uint64_t)))

struct priority_mqueue {
	struct list_head queue[TASK_PRIORITY_NUM];
	uint64_t bitmask[PRIQ_BITMAP_SIZE];
};

struct ready_queue {
	struct task *next_task;
	struct priority_mqueue run_queue;
};

struct per_cpu {
	/* interrupt related */
	uint32_t irq_nested_cnt;
	void *irq_stack_ptr;
	uint32_t irq_stack_size;

	/* task related */
	struct task *current_task;
	struct task *idle_task;

	/* scheduling related */
	struct ready_queue ready_queue;
};

struct kernel {
	struct per_cpu percpus[CONFIG_CPUS_MAX_NUM];
	bool pend_ipi;
};

#endif
