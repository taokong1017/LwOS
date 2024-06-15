#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <types.h>
#include <list.h>
#include <menuconfig.h>
#include <task.h>
#include <spin_lock.h>

#define MASK_NBITS 64
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define PRIQ_BITMAP_SIZE (DIV_ROUND_UP(TASK_PRIORITY_NUM, MASK_NBITS))

struct priority_mqueue {
	struct list_head queues[TASK_PRIORITY_NUM];
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
	struct timer_queue timer_queue;
};

struct kernel {
	struct per_cpu percpus[CONFIG_CPUS_MAX_NUM];
};

void prio_mq_add(struct priority_mqueue *prio_mq, struct task *task);
void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task);
struct task *prio_mq_best(struct priority_mqueue *prio_mq);

#endif
