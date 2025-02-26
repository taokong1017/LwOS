#include <types.h>
#include <task.h>
#include <errno.h>
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
#include <task_cmd.h>
#include <user_space.h>

#define TASK_TO_ID(task) ((task_id_t)task)
#define TASK_TAG "TASK"

SPIN_LOCK_DECLARE(sched_spinlocker);
extern uint64_t mask_trailing_zeros(uint64_t mask);

/*
 * when the delay task exceeds timeout, it will be added to ready
 * queue and then the task will be scheduled.
 *
 * Here, we don't need to use shched locker to protect task status,
 * because the task which put in the percpu timer queue is already
 * locked using the interface spin_lock_save.
 */
bool task_delay_timeout(struct timeout *timeout) {
	uint32_t usable_affi = 0;
	struct task *task = container_of(timeout, struct task, timeout);
	if (TASK_IS_PEND(task)) {
		task->status = TASK_STATUS_READY;
		task->is_timeout = true;
		usable_affi = task->cpu_affi & percpu_idle_mask_get();
		task->cpu_id =
			mask_trailing_zeros(usable_affi ? usable_affi : task->cpu_affi);
		sched_ready_queue_add(task->cpu_id, task);
		return true;
	} else {
		task->status &= ~TASK_STATUS_PEND;
		return false;
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
	task->sig = TASK_SIG_NONE;
	task->stack_ptr = stack_ptr;
	task->stack_size = stack_size;
	task->cpu_affi = TASK_CPU_DEFAULT_AFFI;
	task->cpu_id = 0;
	task->flag = flag;
	task->entry = entry;
	task->is_timeout = false;
	task->args[0] = arg0;
	task->args[1] = arg1;
	task->args[2] = arg2;
	task->args[3] = arg3;
	task->timeout.func = task_delay_timeout;
	INIT_LIST_HEAD(&task->timeout.node);
	INIT_LIST_HEAD(&task->task_list);
	INIT_LIST_HEAD(&task->pend_list);
	task_inherit_perm(task);
}

void task_reset(struct task *task) {
	if (TASK_IS_READY(task) || TASK_IS_RUNNING(task)) {
		sched_ready_queue_remove(task->cpu_id, task);
	}
	if (TASK_IS_PEND(task)) {
		timeout_queue_del(&task->timeout, task->cpu_id);
	}
	task->status = TASK_STATUS_STOP;
	task->is_timeout = false;
	task->lock_cnt = 0;
	arch_task_init(task->id);
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

static void task_service_notify(task_id_t id, enum task_cmd_type cmd_type) {
	struct per_cpu *percpu = current_percpu_get();
	struct task_cmd cmd = {.id = id, .cmd = cmd_type, .data = NULL};

	msgq_send(percpu->msgq_id, &cmd, sizeof(cmd), MSGQ_WAIT_FOREVER);
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
	uint32_t usable_affi = 0;

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if ((cpu_affi > TASK_CPU_AFFI_MASK) || !cpu_affi) {
		return ERRNO_TASK_CPU_AFFI_INAVLID;
	}

	key = sched_spin_lock();
	task->cpu_affi = cpu_affi;
	usable_affi = cpu_affi & percpu_idle_mask_get();

	if (TASK_IS_RUNNING(task)) {
		if (current_task_get() == task) {
			task_sched_locked();
		} else {
			log_info(TASK_TAG, "set running task affi on other cpu\n");
			task->sig = TASK_SIG_AFFI;
			smp_sched_notify();
		}
	}

	if (TASK_IS_READY(task)) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->cpu_id =
			mask_trailing_zeros(usable_affi ? usable_affi : cpu_affi);
		sched_ready_queue_add(task->cpu_id, task);
		log_info(TASK_TAG, "set ready %s task affi cpu%d\n", task->name,
				 task->cpu_id);
	}

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
	uint32_t cur_cpu_id = arch_cpu_id_get();
	uint64_t current_ticks = current_ticks_get();

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
	if (task->cpu_id == cur_cpu_id) {
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

errno_t task_stop(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t cur_cpu_id = arch_cpu_id_get();
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

	if (TASK_IS_RUNNING(task)) {
		if (task->cpu_id == cur_cpu_id) {
			sched_spin_unlock(key);
			task_service_notify(task->id, TASK_CMD_STOP);
			key = sched_spin_lock();
		} else {
			task->sig = TASK_SIG_STOP;
			smp_sched_notify();
		}
	} else {
		task_reset(task);
	}

	sched_spin_unlock(key);

	return OK;
}

errno_t task_stop_self() { return task_stop(task_self_id()); }

errno_t task_resume(task_id_t task_id) {
	uint32_t key = 0;
	uint32_t usable_affi = 0;
	struct task *task = ID_TO_TASK(task_id);
	struct task *current_task = current_task_get();

	if (!task || task->id != task_id) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (task == current_task) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
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
	struct task *current_task = current_task_get();

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
	if (TASK_IS_RUNNING(task) && task->cpu_id != arch_cpu_id_get()) {
		task->sig = TASK_SIG_SUSPEND;
		smp_sched_notify();
		sched_spin_unlock(key);
		return ERRNO_TASK_WILL_SUSPEND;
	}

	/* task is running on the current cpu */
	if (task->status & TASK_STATUS_RUNNING) {
		task->status &= ~TASK_STATUS_RUNNING;
		sched_ready_queue_remove(task->cpu_id, task);
	}

	task->status |= TASK_STATUS_SUSPEND;
	if (task == current_task) {
		task_sched_locked();
	}
	sched_spin_unlock(key);

	return OK;
}

errno_t task_suspend_self() { return task_suspend(task_self_id()); }

errno_t task_delay(uint64_t ticks) {
	struct task *task = current_task_get();
	uint64_t current_ticks = current_ticks_get();
	uint32_t key = 0;

	if (is_in_irq()) {
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	if (ticks == TASK_WAIT_FOREVER) {
		return ERRNO_TASK_INVALID_TIMEOUT;
	}

	if (task->flag & TASK_FLAG_SYSTEM) {
		return ERRNO_TASK_OPERATE_INVALID;
	}

	key = sched_spin_lock();
	if (TASK_IS_LOCKED(task)) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IS_LOCKED;
	}

	if (task->status != TASK_STATUS_RUNNING) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (ticks >= 0) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->status = TASK_STATUS_PEND;
		task->timeout.deadline_ticks = current_ticks + ticks;
		timeout_queue_add(&task->timeout, task->cpu_id);
	}

	log_debug(TASK_TAG, "%s delay %d ticks\n", task->name, ticks);
	task->is_timeout = false;
	task_sched_locked();
	sched_spin_unlock(key);

	return OK;
}

void task_lock() {
	uint32_t key = 0;
	struct task *task = NULL;

	key = sched_spin_lock();
	task = current_task_get();
	task->lock_cnt++;
	sched_spin_unlock(key);

	return;
}

void task_unlock() {
	uint32_t key = 0;
	struct task *task = NULL;

	key = sched_spin_lock();
	task = current_task_get();

	if (task->lock_cnt > 0) {
		task->lock_cnt--;
	}

	if (task->lock_cnt == 0) {
		task_sched_locked();
	}
	sched_spin_unlock(key);

	return;
}

errno_t task_wait_locked(struct wait_queue *wq, uint64_t ticks,
						 bool need_sched) {
	struct task *task = current_task_get();
	uint32_t cur_cpuid = arch_cpu_id_get();

	if (!wq) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!need_sched) {
		return ERRNO_TASK_NO_SCHEDLE;
	}

	sched_ready_queue_remove(task->cpu_id, task);
	task->status = TASK_STATUS_PEND;
	if (ticks != TASK_WAIT_FOREVER) {
		task->timeout.deadline_ticks = current_ticks_get() + ticks;
		timeout_queue_add(&task->timeout, task->cpu_id);
	}
	list_add_tail(&task->pend_list, &wq->wait_list);
	if (task->cpu_id == cur_cpuid) {
		task_sched_locked();
	} else {
		smp_sched_notify();
	}
	if (task->is_timeout) {
		task->is_timeout = false;
		return ERRNO_TASK_WAIT_TIMEOUT;
	}

	return OK;
}

errno_t task_wakeup_locked(struct wait_queue *wq) {
	uint32_t usable_affi = 0;
	struct task *task = NULL;
	uint32_t cur_cpuid = arch_cpu_id_get();

	if (!wq) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!list_empty(&wq->wait_list)) {
		task = list_first_entry(&wq->wait_list, struct task, pend_list);
		list_del_init(wq->wait_list.next);
		task->status = TASK_STATUS_READY;
		usable_affi = task->cpu_affi & percpu_idle_mask_get();
		task->cpu_id =
			mask_trailing_zeros(usable_affi ? usable_affi : task->cpu_affi);
		sched_ready_queue_add(task->cpu_id, task);
		task->is_timeout = false;
		if ((task->cpu_id == cur_cpuid) && (!is_in_irq())) {
			task_sched_locked();
		} else {
			smp_sched_notify();
		}
	}

	return OK;
}

bool task_sig_handle() {
	bool need_sched = false;
	struct task *cur_task = current_task_get();
	struct per_cpu *per_cpu = current_percpu_get();

	if (spin_lock_is_locked(&sched_spinlocker)) {
		return need_sched;
	}

	/* task signal is set by other cpu */
	if (cur_task->sig == TASK_SIG_SUSPEND) {
		cur_task->sig &= ~TASK_SIG_SUSPEND;
		task_suspend(cur_task->id);
	}

	if (cur_task->sig == TASK_SIG_STOP) {
		cur_task->sig &= ~TASK_SIG_STOP;
		task_stop(cur_task->id);
	}

	if (cur_task->sig == TASK_SIG_AFFI) {
		cur_task->sig &= ~TASK_SIG_AFFI;
		task_cpu_affi_set(cur_task->id, cur_task->cpu_affi);
	}

	/* if the task is waiting for the signal, wake it up */
	if (TASK_IS_LOCKED(cur_task)) {
		per_cpu->pend_sched = true;
	} else {
		if (per_cpu->pend_sched) {
			per_cpu->pend_sched = false;
			need_sched = true;
		}
	}

	return need_sched;
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
