#include <types.h>
#include <task.h>
#include <kerrno.h>
#include <mem_mgr.h>
#include <string.h>
#include <arch_task.h>
#include <arch_spinlock.h>
#include <task_sched.h>
#include <tick.h>
#include <irq.h>
#include <log.h>
#include <cpu.h>
#include <smp.h>
#include <msgq.h>
#include <user_space.h>

#define TASK_TO_ID(task) ((task_id_t)task)
#define TASK_TAG "TASK"

SPIN_LOCK_DECLARE(sched_spinlocker);
extern uint64_t mask_trailing_zeros(uint64_t mask);

void task_delay_timeout(struct timeout *timeout) {
	uint32_t usable_affi = 0;
	struct task *task = container_of(timeout, struct task, timeout);
	if (TASK_IS_PEND(task)) {
		task->status = TASK_STATUS_READY;
		task->is_timeout = true;
		usable_affi = task->cpu_affi & percpu_idle_mask_get();
		task->cpu_id =
			mask_trailing_zeros(usable_affi ? usable_affi : task->cpu_affi);
		sched_ready_queue_add(task->cpu_id, task);
	} else {
		task->status &= ~TASK_STATUS_PEND;
	}
}

static errno_t task_params_check(task_id_t *task_id, const char *name,
								 task_entry_func entry, void *arg0, void *arg1,
								 void *arg2, void *arg3, uint32_t stack_size,
								 uint32_t flag) {
	if (!task_id) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!name) {
		return ERRNO_TASK_NAME_EMPTY;
	}

	if (!entry) {
		return ERRNO_TASK_ENTRY_NULL;
	}

	if (stack_size < TASK_STACK_DEFAULT_SIZE) {
		return ERRNO_TASK_STKSZ_INVALID;
	}

	if (!(flag & TASK_FLAG_MASK)) {
		return ERRNO_TASK_FLAG_INVALID;
	}

	return OK;
}

static void task_inherit_perm(struct task *task) {
	struct task *current_task = current_task_get();

	if ((current_task != NULL) && (TASK_IS_PERM_INHERIT(task))) {
		task->mem_domain = current_task->mem_domain;
	}
}

static void task_init(struct task *task, const char *name,
					  task_entry_func entry, void *arg0, void *arg1, void *arg2,
					  void *arg3, void *stack_ptr, uint32_t stack_size,
					  uint32_t flag) {
	memset(task, 0, sizeof(struct task));
	task->is_idle_task = false;
	task->id = TASK_TO_ID(task);
	strncpy(task->name, name, TASK_NAME_LEN);
	task->name[TASK_NAME_LEN - 1] = '\0';
	task->status = TASK_STATUS_STOP;
	task->stack_ptr = stack_ptr;
	task->stack_size = stack_size;
	task->cpu_affi = TASK_CPU_DEFAULT_AFFI;
	task->flag = flag;
	task->entry = entry;
	task->is_timeout = false;
	task->args[0] = arg0;
	task->args[1] = arg1;
	task->args[2] = arg2;
	task->args[3] = arg3;
	task->timeout.func = task_delay_timeout;
	INIT_LIST_HEAD(&task->timeout.node);
	INIT_LIST_HEAD(&task->task_node);
	INIT_LIST_HEAD(&task->halt_queue.wait_list);
	INIT_LIST_HEAD(&task->join_queue.wait_list);
	task_inherit_perm(task);
}

static void task_reset(struct task *task) {
	if (TASK_IS_READY(task) || TASK_IS_RUNNING(task)) {
		sched_ready_queue_remove(task->cpu_id, task);
	}
	if (TASK_IS_PEND(task)) {
		timeout_queue_del(&task->timeout, task->cpu_id);
	}
	task->is_timeout = false;
	arch_task_init(task->id);
	task->status = TASK_STATUS_STOP;
}

errno_t task_create(task_id_t *task_id, const char *name, task_entry_func entry,
					void *arg0, void *arg1, void *arg2, void *arg3,
					uint32_t stack_size, uint32_t flag) {
	void *stack_limit = NULL;
	void *stack_ptr = NULL;
	struct task *task = NULL;

	if (is_in_irq()) {
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	errno_t err = task_params_check(task_id, name, entry, arg0, arg1, arg2,
									arg3, stack_size, flag);
	if (err) {
		return err;
	}

	stack_limit = kmalloc(stack_size);
	stack_ptr = (void *)((uintptr_t)stack_limit + stack_size);
	if (!stack_limit) {
		log_fatal(TASK_TAG, "allocate stack of task %s failed without memory\n",
				  name);
		return ERRNO_TASK_NO_MEMORY;
	}

	task = (struct task *)kmalloc(sizeof(struct task));
	if (!task) {
		kfree(stack_limit);
		log_fatal(TASK_TAG, "allocate task %s failed without memory\n", name);
		return ERRNO_TASK_NO_MEMORY;
	}

	task_init(task, name, entry, arg0, arg1, arg2, arg3, stack_ptr, stack_size,
			  flag);
	arch_task_init(task->id);

	*task_id = task->id;

	return OK;
}

void task_kernel_entry_point(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);

	spin_unlock(&sched_spinlocker);
	arch_irq_unlock();
	log_debug(TASK_TAG, "kernel task %s starts from entry point\n", task->name);
	task->entry(task->args[0], task->args[1], task->args[2], task->args[3]);
	task_suspend_self();
	forever();
}

errno_t task_priority_set(task_id_t task_id, uint32_t prioriy) {
	uint32_t key = 0;
	struct task *task = ID_TO_TASK(task_id);

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (prioriy > TASK_PRIORITY_HIGHEST) {
		return ERRNO_TASK_PRIOR_ERROR;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	if (TASK_IS_STOP(task)) {
		task->priority = prioriy;
		sched_spin_unlock(key);
		return OK;
	}

	if (TASK_IS_READY(task) || TASK_IS_RUNNING(task)) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->priority = prioriy;
		sched_ready_queue_add(task->cpu_id, task);
	} else {
		task->priority = prioriy;
	}

	task_sched_locked();
	sched_spin_unlock(key);

	return OK;
}

errno_t task_priority_get(task_id_t task_id, uint32_t *prioriy) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (!prioriy) {
		return ERRNO_TASK_PRIORITY_EMPTY;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}
	*prioriy = task->priority;
	sched_spin_unlock(key);

	return OK;
}

errno_t task_cpu_affi_set(task_id_t task_id, uint32_t cpu_affi) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if ((cpu_affi > TASK_CPU_AFFI_MASK) || !cpu_affi) {
		return ERRNO_TASK_CPU_AFFI_INAVLID;
	}

	key = sched_spin_lock();
	task->cpu_affi = cpu_affi;
	sched_spin_unlock(key);

	return OK;
}

errno_t task_cpu_bind(task_id_t task_id, uint32_t cpu) {
	if (cpu >= CONFIG_CPUS_MAX_NUM) {
		return ERRNO_TASK_INVALID_CPU_ID;
	}

	return task_cpu_affi_set(task_id, BIT(cpu));
}

errno_t task_cpu_affi_get(task_id_t task_id, uint32_t *cpu_affi) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!cpu_affi) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	*cpu_affi = task->cpu_affi;
	sched_spin_unlock(key);

	return OK;
}

errno_t task_start(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;
	uint64_t current_ticks = 0;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	if (task->status != TASK_STATUS_STOP) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->mem_domain == NULL) {
		sched_spin_unlock(key);
		return ERRNO_TASK_MEM_DOMAIN_NULL;
	}

	task->cpu_id = mask_trailing_zeros(task->cpu_affi);
	current_ticks = current_ticks_get();
	if (task->cpu_id == arch_cpu_id_get()) {
		task->status = TASK_STATUS_READY;
		sched_ready_queue_add(task->cpu_id, task);
		task_sched_locked();
	} else {
		task->status = TASK_STATUS_PEND;
		task->timeout.deadline_ticks = current_ticks;
		timeout_queue_add(&task->timeout, task->cpu_id);
	}
	sched_spin_unlock(key);

	return OK;
}

task_id_t task_self_id() {
	task_id_t task_id = TASK_INVALID_ID;
	uint32_t key = 0;

	key = sched_spin_lock();
	task_id = current_task_get()->id;
	sched_spin_unlock(key);

	return task_id;
}

errno_t task_unpend_no_timeout(struct wait_queue *wq, struct task *task) {
	struct task *pos = NULL, *next = NULL;

	if (!wq || !task) {
		return ERRNO_TASK_PTR_NULL;
	}

	list_for_each_entry_safe(pos, next, &wq->wait_list, task_node) {
		if (pos == task) {
			list_del_init(&task->task_node);
			break;
		}
	}
	wq = NULL;

	return OK;
}

static errno_t task_unpend_all_locked(struct wait_queue *wq) {
	struct task *task = NULL, *next = NULL;

	if (!wq) {
		return ERRNO_TASK_PTR_NULL;
	}

	list_for_each_entry_safe(task, next, &wq->wait_list, task_node) {
		task_unpend_no_timeout(task->pended_on, task);
		list_del_init(&task->task_node);
		timeout_queue_del(&task->timeout, task->cpu_id);
		task->is_timeout = false;
		task->status = TASK_STATUS_READY;
		sched_ready_queue_add(task->cpu_id, task);
	}

	return OK;
}

errno_t task_stop(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (task->flag & TASK_FLAG_SYSTEM) {
		sched_spin_unlock(key);
		return ERRNO_TASK_OPERATE_INVALID;
	}

	if (TASK_IS_STOP(task)) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	/* the task is running on the other cpu */
	if (TASK_IS_RUNNING(task) && task != current_task_get()) {
		task->status = TASK_STATUS_STOPING;
		task_wait_locked(&task->halt_queue, TASK_WAIT_FOREVER);
		sched_spin_unlock(key);
		return OK;
	}

	task_reset(task);
	if (task->pended_on) {
		task_unpend_no_timeout(task->pended_on, task);
	}
	task_unpend_all_locked(&task->halt_queue);
	task_unpend_all_locked(&task->join_queue);
	if (task->cpu_id == arch_cpu_id_get()) {
		task_sched_locked();
	}
	sched_spin_unlock(key);

	return OK;
}

errno_t task_stop_self() { return task_stop(task_self_id()); }

errno_t task_resume(task_id_t task_id) {
	uint32_t key = 0;
	uint32_t usable_affi = 0;
	struct task *task = ID_TO_TASK(task_id);
	struct task *current_task = NULL;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	current_task = current_task_get();
	if (task == current_task) {
		sched_spin_unlock(key);
		return ERRNO_TASK_ID_INVALID;
	}

	if (task->status & TASK_STATUS_STOP) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if ((task->status & TASK_STATUS_RUNNING) ||
		(task->status & TASK_STATUS_READY)) {
		sched_spin_unlock(key);
		return OK;
	}

	if (task->status & TASK_STATUS_PEND) {
		task->status &= ~TASK_STATUS_SUSPEND;
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	task->status &= ~TASK_STATUS_SUSPEND;
	task->status |= TASK_STATUS_READY;
	usable_affi = task->cpu_affi & percpu_idle_mask_get();
	task->cpu_id =
		mask_trailing_zeros(usable_affi ? usable_affi : task->cpu_affi);
	sched_ready_queue_add(task->cpu_id, task);
	task_sched_locked();
	sched_spin_unlock(key);

	return OK;
}

errno_t task_suspend(task_id_t task_id) {
	uint32_t key = 0;
	struct task *task = ID_TO_TASK(task_id);

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (task->status & TASK_STATUS_SUSPEND) {
		sched_spin_unlock(key);
		return OK;
	}

	if (TASK_IS_STOP(task)) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->status & TASK_STATUS_READY) {
		task->status &= ~TASK_STATUS_READY;
		sched_ready_queue_remove(task->cpu_id, task);
	}

	/* The task is running on the other cpu */
	if (TASK_IS_RUNNING(task) && task != current_task_get()) {
		task->status = TASK_STATUS_SUSPENDING;
		task_wait_locked(&task->halt_queue, TASK_WAIT_FOREVER);
		sched_spin_unlock(key);
		return OK;
	}

	/* task is running on the current cpu */
	if (task->status & TASK_STATUS_RUNNING) {
		task->status &= ~TASK_STATUS_RUNNING;
		sched_ready_queue_remove(task->cpu_id, task);
	}

	if (task->pended_on) {
		task_unpend_no_timeout(task->pended_on, task);
	}
	task_unpend_all_locked(&task->halt_queue);
	task->status |= TASK_STATUS_SUSPEND;
	if (task->cpu_id == arch_cpu_id_get()) {
		task_sched_locked();
	}
	sched_spin_unlock(key);

	return OK;
}

errno_t task_suspend_self() { return task_suspend(task_self_id()); }

errno_t task_delay(uint64_t ticks) {
	struct task *task = NULL;
	uint64_t current_ticks = 0;
	uint32_t key = 0;

	if (ticks == TASK_WAIT_FOREVER) {
		return ERRNO_TASK_INVALID_TIMEOUT;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	task = current_task_get();
	if (task->flag & TASK_FLAG_SYSTEM) {
		sched_spin_unlock(key);
		return ERRNO_TASK_OPERATE_INVALID;
	}

	if (task->status != TASK_STATUS_RUNNING) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	sched_ready_queue_remove(task->cpu_id, task);
	task->status = TASK_STATUS_PEND;
	current_ticks = current_ticks_get();
	task->timeout.deadline_ticks = current_ticks + ticks;
	timeout_queue_add(&task->timeout, task->cpu_id);

	log_debug(TASK_TAG, "%s delay %d ticks\n", task->name, ticks);
	task->is_timeout = false;
	task_sched_locked();
	sched_spin_unlock(key);

	return OK;
}

errno_t task_wait_locked(struct wait_queue *wq, uint64_t ticks) {
	struct task *task = current_task_get();

	if (!wq) {
		return ERRNO_TASK_PTR_NULL;
	}

	sched_ready_queue_remove(task->cpu_id, task);
	task->status = TASK_STATUS_PEND;
	if (ticks != TASK_WAIT_FOREVER) {
		task->timeout.deadline_ticks = current_ticks_get() + ticks;
		timeout_queue_add(&task->timeout, task->cpu_id);
	}
	task->pended_on = wq;
	list_add_tail(&task->task_node, &wq->wait_list);
	task_sched_locked();
	if (task->is_timeout) {
		task->is_timeout = false;
		return ERRNO_TASK_WAIT_TIMEOUT;
	}

	return OK;
}

errno_t task_wakeup_locked(struct wait_queue *wq) {
	uint32_t usable_affi = 0;
	struct task *task = NULL;

	if (!wq) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!list_empty(&wq->wait_list)) {
		task = list_first_entry(&wq->wait_list, struct task, task_node);
		list_del_init(wq->wait_list.next);
		task->status = TASK_STATUS_READY;
		usable_affi = task->cpu_affi & percpu_idle_mask_get();
		task->cpu_id =
			mask_trailing_zeros(usable_affi ? usable_affi : task->cpu_affi);
		sched_ready_queue_add(task->cpu_id, task);
		task->is_timeout = false;
		if ((task->cpu_id == arch_cpu_id_get()) && (!is_in_irq())) {
			task_sched_locked();
		}
	}

	return OK;
}

bool task_sched_check() {
	struct per_cpu *per_cpu = current_percpu_get();

	if (spin_lock_is_locked(&sched_spinlocker)) {
		return false;
	}

	/* task does not need to schedule */
	if (per_cpu->pend_sched) {
		per_cpu->pend_sched = false;
		return false;
	}

	return true;
}

errno_t task_mem_domain_add(task_id_t task_id, struct mem_domain *domain) {
	struct task *task = ID_TO_TASK(task_id);

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (!domain) {
		return ERRNO_TASK_MEM_DOMAIN_NULL;
	}

	task->mem_domain = domain;

	return OK;
}

#ifdef CONFIG_USER_SPACE
bool task_is_user(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);

	return task->flag & TASK_FLAG_USER;
}

errno_t task_create_with_stack(task_id_t *task_id, const char *name,
							   task_entry_func entry, void *arg0, void *arg1,
							   void *arg2, void *arg3, struct task *task,
							   void *stack, uint32_t stack_size,
							   uint32_t flag) {
	void *stack_limit = NULL;
	void *stack_ptr = NULL;

	if (is_in_irq()) {
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	errno_t err = task_params_check(task_id, name, entry, arg0, arg1, arg2,
									arg3, stack_size, flag);
	if (err) {
		return err;
	}

	stack_limit = stack;
	stack_ptr = (void *)((uintptr_t)stack_limit + stack_size);
	if (!stack_limit) {
		log_fatal(TASK_TAG, "allocate stack of task %s failed without memory\n",
				  name);
		return ERRNO_TASK_NO_MEMORY;
	}

	if (!task) {
		log_fatal(TASK_TAG, "allocate task %s failed without memory\n", name);
		return ERRNO_TASK_NO_MEMORY;
	}

	task_init(task, name, entry, arg0, arg1, arg2, arg3, stack_ptr, stack_size,
			  flag);
	arch_task_init(task->id);

	*task_id = task->id;

	return OK;
}

void task_user_entry_point(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);

	user_task_sched_unlock();
	task->entry(task->args[0], task->args[1], task->args[2], task->args[3]);
	user_task_suspend(task->id);
}
#endif
