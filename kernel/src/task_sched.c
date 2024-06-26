#include <task.h>
#include <kernel.h>
#include <cpu.h>
#include <task_sched.h>
#include <string.h>
#include <irq.h>
#include <operate_regs.h>
#include <log.h>

#define IDLE_TASK_NAME "idle_task"
#define ROOT_TASK_NAME "main_task"
#define TASK_SCHED_TAG "TASK_SCHED"
#define TASK_IS_LOCKED(task) (task->lock_cnt > 0)
#define current_percpu kernel.percpus[arch_cpu_id_get()]

SPIN_LOCK_DEFINE(sched_spinlock);
extern char __interrupt_stack_start[];
extern char __interrupt_stack_end[];
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

	list_add_tail(&task->node, &prio_mq->queues[pos.prio]);
	prio_mq->bitmask[pos.idx] |= BIT(pos.bit);
}

void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pri = prio_info_get(task->priority);
	struct list_head *queue = &prio_mq->queues[pri.prio];
	struct list_head *pos = NULL;
	struct list_head *next = NULL;

	list_for_each_safe(pos, next, queue) {
		if (pos == &task->node) {
			list_del_init(pos);
			prio_mq->bitmask[pri.idx] &= ~BIT(pri.bit);
		}
	}
}

static uint64_t mask_trailing_zeros(uint64_t mask) {
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
			task = list_first_entry(list, struct task, node);
			break;
		}
	}

	return task;
}

struct task *current_task_get() {
	return current_percpu.current_task;
}

struct per_cpu *current_percpu_get() {
	return &current_percpu;
}

uint32_t sched_spin_lock() {
	uint32_t key = 0;
	spin_lock_save(&sched_spinlock, &key);

	return key;
}

void sched_spin_unlock(uint32_t key) {
	spin_lock_restore(&sched_spinlock, key);
}

void sched_ready_queue_remove(uint32_t cpu_id, struct task *task) {
	prio_mq_remove(&kernel.percpus[cpu_id].ready_queue.run_queue, task);
}

void sched_ready_queue_add(uint32_t cpu_id, struct task *task) {
	prio_mq_add(&kernel.percpus[cpu_id].ready_queue.run_queue, task);
}

bool is_in_irq() { return current_percpu.irq_nested_cnt > 0 ? true : false; }

static void current_task_update(struct task *task) {
	current_percpu.current_task = task;
}

static struct task *next_task_pick_up() {
	return prio_mq_best(&current_percpu.ready_queue.run_queue);
}

static void task_switch(struct task *new, struct task *old) {
	arch_task_context_switch(&new->task_context, &old->task_context);
}

struct stack_info irq_stack_info(struct task *task) {
	uint32_t cpu_id = task->cpu_id;
	struct stack_info irq_stack;

	irq_stack.high = (phys_addr_t)kernel.percpus[cpu_id].irq_stack_ptr;
	irq_stack.low = (phys_addr_t)kernel.percpus[cpu_id].irq_stack_ptr -
					kernel.percpus[cpu_id].irq_stack_size;

	return irq_stack;
}

#include <tick.h>
#include <stdio.h>
uint32_t test() {
	uint32_t d1 = 1;
	uint32_t d2 = 2;
	uint32_t d3 = 3;
	uint32_t d4 = 4;
	uint32_t d5 = 5;
	uint32_t d6 = 6;
	uint32_t d7 = 7;
	uint32_t d8 = 8;
	uint32_t d9 = 9;
	uint32_t d10 = 10;
	uint32_t d11 = 11;
	uint32_t d12 = 12;
	uint32_t d13 = 13;
	uint32_t d14 = 14;
	uint32_t d15 = 15;
	uint32_t d16 = 16;
	uint32_t d17 = 17;
	uint32_t d18 = 18;
	uint32_t d19 = 19;
	uint32_t d20 = 20;
	uint32_t d21 = 21;
	uint32_t d22 = 22;
	uint32_t d23 = 23;
	uint32_t d24 = 24;
	uint32_t sum = 0;

	mdelay(1000);
	sum = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 + d11 + d12 + d13 +
		  d14 + d15 + d16 + d17 + d18 + d19 + d20 + d21 + d22 + d23 + d24;
	return sum;
}

static bool show_Linker(void *cookie, phys_addr_t pc) {
	uint32_t *level = (uint32_t *)cookie;
	printf("%u: 0x%016llx\n", (*level)++, pc);
	return true;
}

static void idle_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	for (;;) {
		uint32_t level = 0;
		uint32_t prioriy = -1;
		task_prority_get(task_self_id(), &prioriy);
		printf("idle task priority %d\n", prioriy);
		arch_stack_walk(show_Linker, &level, current_task_get(), NULL);
		printf("idle task %d\n", test());
	}
}

void idle_task_create() {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, IDLE_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, idle_task_entry, (void *)1, (void *)2,
				(void *)3, (void *)4, TASK_STACK_DEFAULT_SIZE,
				TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_LOWEST;
	task->status = TASK_STATUS_READY;
	current_percpu.idle_task = task;
	sched_ready_queue_add(task->cpu_id, task);
}

bool task_is_idle_task(struct task *task) {
	return current_percpu.idle_task == task ? true : false;
}

void main_task_create() {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, ROOT_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, root_task_entry, NULL, NULL, NULL, NULL,
				TASK_STACK_DEFAULT_SIZE, TASK_FLAG_KERNEL);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_HIGHEST;
	task->status = TASK_STATUS_RUNNING;
	sched_ready_queue_add(task->cpu_id, task);
	current_task_update(task);
	arch_main_task_switch(task_id);
}

void task_resched() {
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	}

	current_task->status &= ~TASK_STATUS_RUNNING;
	next_task->status = TASK_STATUS_RUNNING;
	current_task_update(next_task);
	task_switch(next_task, current_task);
}

void task_irq_resched() {
	uint32_t key = 0;
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	}

	if (spin_lock_is_locked(&sched_spinlock)) {
		log_err(TASK_SCHED_TAG, "the sched_spinlock is locked\n");
		spin_lock_dump(&sched_spinlock);
		forever();
	}

	if (TASK_IS_LOCKED(current_task)) {
		return;
	}

	key = sched_spin_lock();
	current_task->status &= ~TASK_STATUS_RUNNING;
	next_task->status = TASK_STATUS_RUNNING;
	current_task_update(next_task);
	task_switch(next_task, current_task);
	sched_spin_unlock(key);

	return;
}

void task_sched_init() {
	int32_t i = 0;

	memset((void *)&current_percpu, 0, sizeof(struct per_cpu));
	for (i = 0; i < TASK_PRIORITY_NUM; i++) {
		INIT_LIST_HEAD(&current_percpu.ready_queue.run_queue.queues[i]);
	}
	INIT_LIST_HEAD(&current_percpu.timer_queue.queue);
	write_tpidrro_el0((uint64_t)current_percpu_get());
	current_percpu_get()->irq_stack_ptr = (void *)__interrupt_stack_end;
	current_percpu_get()->irq_stack_size = 0x1000;

	idle_task_create();
	main_task_create();
	code_unreachable();
}
