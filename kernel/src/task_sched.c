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
extern void main_task_entry(void *arg0, void *arg1, void *arg2, void *arg3);

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

	list_add_tail(&task->task_list, &prio_mq->queues[pos.prio]);
	prio_mq->bitmask[pos.idx] |= BIT(pos.bit);
}

void prio_mq_remove(struct priority_mqueue *prio_mq, struct task *task) {
	struct prio_info pri = prio_info_get(task->priority);
	struct list_head *queue = &prio_mq->queues[pri.prio];
	struct list_head *pos = NULL;
	struct list_head *next = NULL;

	list_for_each_safe(pos, next, queue) {
		if (pos == &task->task_list) {
			list_del_init(pos);
			break;
		}
	}

	if (list_empty(queue)) {
		prio_mq->bitmask[pri.idx] &= ~BIT(pri.bit);
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
			task = list_first_entry(list, struct task, task_list);
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
				(void *)3, (void *)4, TASK_STACK_DEFAULT_SIZE,
				TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_LOWEST;
	task->status = TASK_STATUS_READY;
	sched_ready_queue_add(task->cpu_id, task);
}

void main_task_create() {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, ROOT_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, main_task_entry, NULL, NULL, NULL, NULL,
				TASK_STACK_DEFAULT_SIZE, TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_HIGHEST;
	task->status = TASK_STATUS_RUNNING;
	sched_ready_queue_add(task->cpu_id, task);
	current_task_update(task);
	arch_main_task_switch(task_id);
}

void task_locked_sched() {
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

void task_unlocked_sched() {
	uint32_t key = 0;
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	}

	if (spin_lock_is_locked(&sched_spinlock)) {
		spin_lock_dump(&sched_spinlock);
		log_fatal(TASK_SCHED_TAG, "%s: the sched lock %s is locked\n", __func__,
				  sched_spinlock.name);
		code_unreachable();
	}

	if (TASK_IS_LOCKED(current_task)) {
		return;
	}

	key = sched_spin_lock();

	current_task->status = TASK_STATUS_READY;
	sched_ready_queue_remove(current_task->cpu_id, current_task);
	sched_ready_queue_add(current_task->cpu_id, current_task);

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
	memset((void *)(current_percpu_get()->irq_stack_ptr -
					current_percpu_get()->irq_stack_size),
		   0, current_percpu_get()->irq_stack_size);

	idle_task_create();
	main_task_create();
	code_unreachable();
}
