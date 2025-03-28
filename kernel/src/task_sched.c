#include <task.h>
#include <kernel.h>
#include <cpu.h>
#include <task_sched.h>
#include <string.h>
#include <irq.h>
#include <operate_regs.h>
#include <log.h>
#include <percpu.h>
#include <msgq.h>
#include <arch_task.h>
#include <smp.h>

#define IDLE_TASK_NAME "idle_task"
#define ROOT_TASK_NAME "main_task"
#define SYSTEM_TASK_NAME "system_task"
#define TASK_SCHED_TAG "TASK_SCHED"
#define TASK_SCHED_LOCKER "SCHED_SPIN_LOCKER"

SPIN_LOCK_DEFINE(sched_spinlocker, TASK_SCHED_LOCKER);
extern void main(void *arg0, void *arg1, void *arg2, void *arg3);
extern struct mem_domain kernel_mem_domain;
extern void task_halting_handle(struct task *task);

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

	list_add_tail(&task->task_node, &prio_mq->queues[pos.prio]);
	prio_mq->bitmask[pos.idx] |= BIT(pos.bit);
}

void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pri = prio_info_get(task->priority);
	struct list_head *queue = &prio_mq->queues[pri.prio];
	struct list_head *pos = NULL;
	struct list_head *next = NULL;

	list_for_each_safe(pos, next, queue) {
		if (pos == &task->task_node) {
			list_del_init(pos);
			break;
		}
	}

	if (list_empty(queue)) {
		prio_mq->bitmask[pri.idx] &= ~BIT(pri.bit);
	}
}

uint64_t mask_trailing_zeros(uint64_t mask) {
	uint32_t shift = MASK_NBITS - 1;

	if (mask == 0) {
		return 0;
	}

	while (!(mask & (1ULL << shift))) {
		shift--;
	}

	return shift;
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
			task = list_first_entry(list, struct task, task_node);
			break;
		}
	}

	return task;
}

void sched_ready_queue_dump(uint32_t cpu_id) {
	struct per_cpu *per_cpu = percpu_get(cpu_id);
	struct priority_mqueue *prio_mq = &per_cpu->ready_queue.run_queue;
	struct list_head *queue = NULL;
	uint32_t prio = 0;
	struct task *task = NULL;

	printf("\nCPU %u ready queue:\n", cpu_id);
	for (; prio <= TASK_PRIORITY_HIGHEST; ++prio) {
		queue = &prio_mq->queues[prio];
		if (list_empty(queue)) {
			continue;
		}
		printf("priority %u:\n", prio);
		list_for_each_entry(task, queue, task_node) {
			printf("\t%s\n", task->name);
		}
	}
}

struct task *current_task_get() {
	return current_percpu_get()->current_task;
}

uint32_t sched_spin_lock() {
	uint32_t key = 0;
	spin_lock_save(&sched_spinlocker, &key);

	return key;
}

void sched_spin_unlock(uint32_t key) {
	spin_lock_restore(&sched_spinlocker, key);
}

bool sched_spin_is_locked() { return spin_lock_is_locked(&sched_spinlocker); }

void sched_spin_lock_dump() {
#ifdef CONFIG_SPIN_LOCK_TRACE
	spin_lock_dump(&sched_spinlocker);
#else
	log_info(TASK_SCHED_TAG, "spin lock trace is not enabled\n");
#endif
}

void sched_ready_queue_remove(uint32_t cpu_id, struct task *task) {
	prio_mq_remove(&percpu_get(cpu_id)->ready_queue.run_queue, task);
}

void sched_ready_queue_add(uint32_t cpu_id, struct task *task) {
	prio_mq_add(&percpu_get(cpu_id)->ready_queue.run_queue, task);
}

bool is_in_irq() {
	return current_percpu_get()->irq_nested_cnt > 0 ? true : false;
}

static void current_task_update(struct task *task) {
	current_percpu_get()->current_task = task;
}

static struct task *next_task_pick_up() {
	struct task *task = current_task_get();

	task_halting_handle(task);
	return prio_mq_best(&current_percpu_get()->ready_queue.run_queue);
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

void idle_task_create(uint32_t cpu_id) {
	task_id_t task_id = 0;
	struct task *task = NULL;

	task_create(&task_id, IDLE_TASK_NAME, idle_task_entry, (void *)1, (void *)2,
				(void *)3, (void *)4, TASK_STACK_DEFAULT_SIZE,
				TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->is_idle_task = true;
	task->priority = TASK_PRIORITY_LOWEST - 1;
	task->status = TASK_STATUS_READY;
	task->cpu_affi = TASK_CPU_AFFI(cpu_id);
	task->cpu_id = cpu_id;
	task->mem_domain = &kernel_mem_domain;
	sched_ready_queue_add(cpu_id, task);
}

void main_task_create(uint32_t cpu_id) {
	task_id_t task_id = 0;
	struct task *task = NULL;

	task_create(&task_id, ROOT_TASK_NAME, main, NULL, NULL, NULL, NULL,
				TASK_STACK_DEFAULT_SIZE, TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_HIGHEST;
	task->status = TASK_STATUS_READY;
	task->cpu_affi = TASK_CPU_AFFI(cpu_id);
	task->cpu_id = cpu_id;
	task->mem_domain = &kernel_mem_domain;
	sched_ready_queue_add(cpu_id, task);
}

void task_sched_locked() {
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();
	struct per_cpu *per_cpu = current_percpu_get();
	uint32_t usable_affi = 0;
	uint32_t cpu_id = 0;

	if (!current_task || !next_task) {
		return;
	}

	if (current_task == next_task) {
		return;
	}

	log_debug(TASK_SCHED_TAG, "[cpu %d]current task is %s, next task is %s\n",
			  arch_cpu_id_get(), current_task->name, next_task->name);

	sched_ready_queue_remove(current_task->cpu_id, current_task);
	if (TASK_IS_READY(current_task) || TASK_IS_RUNNING(current_task)) {
		usable_affi = current_task->cpu_affi & percpu_idle_mask_get();
		cpu_id = mask_trailing_zeros(usable_affi ? usable_affi
												 : current_task->cpu_affi);
		current_task->cpu_id = cpu_id;
		sched_ready_queue_add(current_task->cpu_id, current_task);
		current_task->status = TASK_STATUS_READY;
	}

	if (next_task->is_idle_task) {
		per_cpu->is_idle = true;
	} else {
		per_cpu->is_idle = false;
	}
	next_task->status = TASK_STATUS_RUNNING;

	current_task_update(next_task);
	task_switch(next_task, current_task);
}

void task_sched_unlocked() {
	uint32_t key = 0;
	struct task *current_task = NULL;
	struct task *next_task = NULL;
	struct per_cpu *per_cpu = current_percpu_get();
	uint32_t usable_affi = 0;

	key = sched_spin_lock();

	current_task = current_task_get();
	next_task = next_task_pick_up();
	if (!next_task || !current_task) {
		return;
	}

	if (current_task == next_task) {
		sched_spin_unlock(key);
		return;
	}

	current_task->status = TASK_STATUS_READY;
	sched_ready_queue_remove(current_task->cpu_id, current_task);
	usable_affi = current_task->cpu_affi & percpu_idle_mask_get();
	current_task->cpu_id =
		mask_trailing_zeros(usable_affi ? usable_affi : current_task->cpu_affi);
	sched_ready_queue_add(current_task->cpu_id, current_task);

	if (next_task->is_idle_task) {
		per_cpu->is_idle = true;
	} else {
		per_cpu->is_idle = false;
	}
	next_task->status = TASK_STATUS_RUNNING;
	current_task_update(next_task);

	task_switch(next_task, current_task);
	sched_spin_unlock(key);

	return;
}

void task_sched_start() {
	struct task *task = NULL;

	task = next_task_pick_up();
	current_task_update(task);
	arch_main_task_switch(task->id);

	code_unreachable();
}

void task_sched_unlock() {
	spin_unlock(&sched_spinlocker);
	arch_irq_unlock();

	return;
}
