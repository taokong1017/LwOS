#include <task.h>
#include <kernel.h>
#include <cpu.h>
#include <task_sched.h>
#include <string.h>
#include <irq.h>

#define IDLE_TASK_NAME "idle_task"
#define ROOT_TASK_NAME "main_task"
#define current_percpu kernel.percpus[arch_cpu_id_get()]

extern void root_task_entry(void *arg0, void *arg1, void *arg2, void *arg3);

struct prio_info {
	uint32_t prio;
	uint32_t idx;
	uint32_t bit;
};

static struct kernel kernel;

static struct prio_info prio_info_get(uint32_t priority) {
	struct prio_info ret;

	ret.prio = priority;
	ret.idx = priority / MASK_NBITS;
	ret.bit = priority % MASK_NBITS;

	return ret;
}

void prio_mq_add(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pos = prio_info_get(task->priority);

	list_add(&task->node, &prio_mq->queues[pos.prio]);
	prio_mq->bitmask[pos.idx] |= BIT(pos.bit);
}

void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pos = prio_info_get(task->priority);

	list_move(&task->node, &prio_mq->queues[pos.prio]);

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
			task = list_first_entry(list, struct task, node);
			break;
		}
	}

	return task;
}

static struct task *current_task_get() { return current_percpu.current_task; }

static void current_task_update(struct task *task) {
	current_percpu.current_task = task;
}

static struct task *next_task_pick_up() {
	struct task *next_task = NULL;

	next_task = prio_mq_best(&current_percpu.ready_queue.run_queue);
	if (!next_task) {
		next_task = current_percpu.idle_task;
	} else {
		prio_mq_remove(&current_percpu.ready_queue.run_queue, next_task);
	}

	return next_task;
}

static void task_switch(struct task *new, struct task *old) {
	arch_task_context_switch(&new->task_context, &old->task_context);
}

static void idle_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	forever();
}

void idle_task_create() {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, IDLE_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, idle_task_entry, (void *)1, (void *)2,
				(void *)3, (void *)4, TASK_STACK_SIZE_MIN);
	task_prority_set(task_id, TASK_PRIORITY_LOWEST);
	task = ID_TO_TASK(task_id);
	task->status = TASK_STATUS_READY;
	current_percpu.idle_task = task;
}

bool task_is_idle_task(struct task *task) {
	return strncmp(task->name, IDLE_TASK_NAME, strlen(IDLE_TASK_NAME)) ? true
																	   : false;
}

void main_task_create() {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, ROOT_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, root_task_entry, NULL, NULL, NULL, NULL,
				TASK_STACK_SIZE_MIN);
	task_prority_set(task_id, TASK_PRIORITY_HIGHEST);
	task = ID_TO_TASK(task_id);
	task->status = TASK_STATUS_READY;
	current_task_update(task);
	arch_main_task_switch(task_id);
}

void task_irq_resched() {
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	}

	current_task->status = TASK_STATUS_READY;
	if (!task_is_idle_task(current_task)) {
		prio_mq_add(&current_percpu.ready_queue.run_queue, current_task);
	}

	next_task->status = TASK_STATUS_RUNNING;
	current_task_update(next_task);
	task_switch(next_task, current_task);
}

void task_announce() { ; }

void task_sched_init() {
	int32_t i = 0;

	memset((void *)&current_percpu, 0, sizeof(struct per_cpu));
	for (i = 0; i < TASK_PRIORITY_NUM; i++) {
		INIT_LIST_HEAD(&current_percpu.ready_queue.run_queue.queues[i]);
	}
	INIT_LIST_HEAD(&current_percpu.timer_queue.tq);

	idle_task_create();
	main_task_create();
	code_unreachable();
}
