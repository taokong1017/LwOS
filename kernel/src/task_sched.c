#include <task.h>
#include <task_sched.h>

#define BIT(nr) (1U << nr)

struct prio_info {
	uint32_t prio;
	uint32_t idx;
	uint32_t bit;
};

static struct prio_info prio_info_get(uint32_t priority) {
	struct prio_info ret;

	ret.prio = priority;
	ret.idx = priority / MASK_NBITS;
	ret.bit = priority % MASK_NBITS;

	return ret;
}

void prio_mq_add(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pos = prio_info_get(task->priority);

	list_add(&task->entry_node, &prio_mq->queues[pos.prio]);
	prio_mq->bitmask[pos.idx] |= BIT(pos.bit);
}

void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pos = prio_info_get(task->priority);

	list_move(&task->entry_node, &prio_mq->queues[pos.prio]);

	if (list_empty(&prio_mq->queues[pos.prio])) {
		prio_mq->bitmask[pos.idx] &= ~BIT(pos.bit);
	}
}

static uint64_t mask_trailing_zeros(uint64_t mask) {
	uint64_t idx = 0;
	uint32_t shift = 63;

	if (mask == 0) {
		return 0;
	}

	idx = mask;
	while (!(idx & (1ULL << shift))) {
		shift--;
	}

	idx &= (1ULL << (shift + 1)) - 1;

	return idx;
}

struct task *prio_mq_best(struct priority_mqueue *prio_mq) {
	uint64_t bit = 0;
	uint64_t mask = 0;
	uint32_t idx = 0;
	struct task *task = NULL;

	for (int32_t i = 0; i < PRIQ_BITMAP_SIZE; ++i) {
		idx = PRIQ_BITMAP_SIZE - i - 1;
		mask = prio_mq->bitmask[idx];
		bit = mask_trailing_zeros(mask);

		struct list_head *list = &prio_mq->queues[idx * MASK_NBITS + bit];

		if (!list_empty(list)) {
			task = list_first_entry(list, struct task, entry_node);
			break;
		}
	}

	return task;
}
