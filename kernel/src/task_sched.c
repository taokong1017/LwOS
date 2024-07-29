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

#define IDLE_TASK_NAME "idle_task"
#define ROOT_TASK_NAME "main_task"
#define SYSTEM_TASK_NAME "system_task"
#define TASK_SCHED_TAG "TASK_SCHED"
#define TASK_IS_LOCKED(task) (task->lock_cnt > 0)

SPIN_LOCK_DEFINE(sched_spinlock);
extern void main_task_entry(void *arg0, void *arg1, void *arg2, void *arg3);

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
	return current_percpu_get()->current_task;
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
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, IDLE_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, idle_task_entry, (void *)1, (void *)2,
				(void *)3, (void *)4, TASK_STACK_DEFAULT_SIZE,
				TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_LOWEST - 1;
	task->status = TASK_STATUS_READY;
	task->cpu_affi = TASK_CPU_AFFI(cpu_id);
	task->cpu_id = cpu_id;
	sched_ready_queue_add(cpu_id, task);
}

void main_task_create(uint32_t cpu_id) {
	task_id_t task_id = 0;
	struct task *task = NULL;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, ROOT_TASK_NAME, TASK_NAME_LEN);
	task_create(&task_id, task_name, main_task_entry, NULL, NULL, NULL, NULL,
				TASK_STACK_DEFAULT_SIZE, TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_HIGHEST;
	task->status = TASK_STATUS_READY;
	task->cpu_affi = TASK_CPU_AFFI(cpu_id);
	task->cpu_id = cpu_id;
	sched_ready_queue_add(cpu_id, task);
}

static void system_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	msgq_id_t msgq_id = (msgq_id_t)arg0;
	uint8_t msg[SERVICE_MSGQ_SIZE] = {0};
	uint32_t msg_len = SERVICE_MSGQ_SIZE;

	forever() {
		if (!msgq_receive(msgq_id, msg, &msg_len, MSGQ_WAIT_FOREVER)) {
		}
	}

	return;
}

void system_task_create(uint32_t cpu_id) {
	struct per_cpu *percpu = percpu_get(cpu_id);
	task_id_t task_id = 0;
	struct task *task = NULL;

	task_create(&task_id, SYSTEM_TASK_NAME, system_task_entry,
				(void *)percpu->msgq_id, NULL, NULL, NULL,
				TASK_STACK_DEFAULT_SIZE, TASK_DEFAULT_FLAG);
	task = ID_TO_TASK(task_id);
	task->priority = TASK_PRIORITY_HIGHEST;
	task->status = TASK_STATUS_READY;
	task->cpu_affi = TASK_CPU_AFFI(cpu_id);
	task->cpu_id = cpu_id;
	sched_ready_queue_add(cpu_id, task);
}

void task_sched_locked() {
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	} else {
		log_debug(TASK_SCHED_TAG, "current task is %s, next task is %s\n",
				  current_task->name, next_task->name);
	}

	current_task->status &= ~TASK_STATUS_RUNNING;
	next_task->status = TASK_STATUS_RUNNING;
	current_task_update(next_task);
	task_switch(next_task, current_task);
}

void task_sched_unlocked() {
	uint32_t key = 0;
	struct task *current_task = current_task_get();
	struct task *next_task = next_task_pick_up();

	if (current_task == next_task) {
		return;
	}

	if (TASK_IS_LOCKED(current_task)) {
		return;
	}

	if (spin_lock_is_locked(&sched_spinlock)) {
		log_info(TASK_SCHED_TAG, "%s: the sched lock %s is locked\n", __func__,
				 sched_spinlock.name);
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

void task_sched_start() {
	struct task *task = NULL;

	task = next_task_pick_up();
	current_task_update(task);
	arch_main_task_switch(task->id);

	code_unreachable();
}