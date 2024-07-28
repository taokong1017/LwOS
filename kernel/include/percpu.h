#ifndef __PERCPU_H__
#define __PERCPU_H__

#include <types.h>
#include <task.h>

#define MASK_NBITS 64
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define PRIQ_BITMAP_SIZE (DIV_ROUND_UP(TASK_PRIORITY_NUM, MASK_NBITS))

struct priority_mqueue {
	struct list_head queues[TASK_PRIORITY_NUM];
	uint64_t bitmask[PRIQ_BITMAP_SIZE];
};

struct ready_queue {
	struct priority_mqueue run_queue;
};

struct per_cpu {
	bool pend_sched;

	/* interrupt related */
	uint32_t irq_nested_cnt;
	void *irq_stack_ptr;
	uint32_t irq_stack_size;

	/* task related */
	struct task *current_task;

	/* scheduling related */
	struct ready_queue ready_queue;
	struct timer_queue timer_queue;
};

struct stack_info irq_stack_info(uint32_t cpu_id);
void percpu_init(uint32_t cpu_id);

#endif
