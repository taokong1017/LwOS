#include <types.h>
#include <task.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <arch_task.h>
#include <arch_spinlock.h>
#include <task_sched.h>
#include <tick.h>
#include <irq.h>

#define ALIGN(start, align) ((start + align - 1) & ~(align - 1))
#define TASK_TO_ID(task) ((task_id_t)task)
#define TASK_SCHED_LOCKED(task) (task->lock_cnt > 1)
#define TASK_TAG "Task"

extern struct spinlock sched_spinlock;
extern void task_unlocked_sched();

/*
 * when the delay task exceeds timeout, it will be added to ready
 * queue and then the task will be scheduled.
 *
 * Here, we don't need to use shched locker to protect task status,
 * because the task which put in the percpu timer queue is already
 * locked using the interface spin_lock_save.
 */
void task_delay_timeout(struct timeout *timeout) {
	struct task *task = container_of(timeout, struct task, timeout);
	if (task->status == TASK_STATUS_PEND) {
		task->status = TASK_STATUS_READY;
		sched_ready_queue_add(task->cpu_id, task);
	} else {
		task->status &= ~TASK_STATUS_PEND;
	}
}

static errno_t task_params_check(task_id_t *task_id,
								 const char name[TASK_NAME_LEN],
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

static void task_init(struct task *task, const char name[TASK_NAME_LEN],
					  task_entry_func entry, void *arg0, void *arg1, void *arg2,
					  void *arg3, void *stack_ptr, uint32_t stack_size,
					  uint32_t flag) {
	memset(task, 0, sizeof(struct task));
	task->id = TASK_TO_ID(task);
	memcpy(task->name, name, TASK_NAME_LEN);
	task->status = TASK_STATUS_STOP;
	task->stack_ptr = stack_ptr;
	task->stack_size = stack_size;
	task->cpu_affi = TASK_CPU_DEFAULT_AFFI;
	task->cpu_id = 0;
	task->flag = flag;
	task->entry = entry;
	task->args[0] = arg0;
	task->args[1] = arg1;
	task->args[2] = arg2;
	task->args[3] = arg3;
	task->timeout.func = task_delay_timeout;
	INIT_LIST_HEAD(&task->timeout.node);
}

static void task_reset(struct task *task) {
	task->status = TASK_STATUS_STOP;
	INIT_LIST_HEAD(&task->timeout.node);
	arch_task_init(task->id);
}

errno_t task_create(task_id_t *task_id, const char name[TASK_NAME_LEN],
					task_entry_func entry, void *arg0, void *arg1, void *arg2,
					void *arg3, uint32_t stack_size, uint32_t flag) {
	uint32_t align_size = 0;
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

	align_size = ALIGN(stack_size, TASK_STACK_SIZE_ALIGN);
	stack_limit = mem_alloc_align(align_size, TASK_STACK_ADDR_ALIGN);
	stack_ptr = stack_limit + align_size;
	task = (struct task *)mem_alloc(sizeof(struct task));
	if (!stack_limit || !task) {
		return ERRNO_TASK_NO_MEMORY;
	}

	task_init(task, name, entry, arg0, arg1, arg2, arg3, stack_ptr, align_size,
			  flag);
	arch_task_init(task->id);

	*task_id = task->id;

	return OK;
}

void task_entry_point(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);

	spin_unlock(&sched_spinlock);
	arch_irq_unlock();
	task->entry(task->args[0], task->args[1], task->args[2], task->args[3]);
}

errno_t task_prority_set(task_id_t task_id, uint32_t prioriy) {
	uint32_t key = 0;
	struct task *task = ID_TO_TASK(task_id);

	if (!task) {
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

	if (task->status == TASK_STATUS_STOP) {
		task->priority = prioriy;
		sched_spin_unlock(key);
		return OK;
	}

	if ((task->status == TASK_STATUS_READY) ||
		(task->status == TASK_STATUS_RUNNING)) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->priority = prioriy;
		sched_ready_queue_add(task->cpu_id, task);
	} else {
		task->priority = prioriy;
	}

	task_locked_sched();
	sched_spin_unlock(key);

	return OK;
}

errno_t task_prority_get(task_id_t task_id, uint32_t *prioriy) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!task) {
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

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (cpu_affi > TASK_CPU_AFFI_MASK) {
		return ERRNO_TASK_CPU_AFFI_INAVLID;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	// TO DO

	sched_spin_unlock(key);

	return OK;
}

errno_t task_cpu_affi_get(task_id_t task_id, uint32_t *cpu_affi) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!cpu_affi) {
		return ERRNO_TASK_PTR_NULL;
	}

	if (!task) {
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

	if (!task) {
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

	task->status = TASK_STATUS_READY;
	sched_ready_queue_add(task->cpu_id, task);
	task_locked_sched();
	sched_spin_unlock(key);

	return OK;
}

task_id_t task_self_id() {
	task_id_t task_id = 0;
	uint32_t key = 0;

	key = sched_spin_lock();

	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	task_id = current_task_get()->id;
	sched_spin_unlock(key);

	return task_id;
}

errno_t task_stop(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t key = 0;

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	key = sched_spin_lock();
	if (is_in_irq()) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	if (task->flag & TASK_FLAG_SYSTEM) {
		sched_spin_unlock(key);
		return ERRNO_TASK_OPERATE_INVALID;
	}

	if (task->status == TASK_STATUS_STOP) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->status == TASK_STATUS_READY) {
		sched_ready_queue_remove(task->cpu_id, task);
	}

	if (task->status & TASK_STATUS_SUSPEND) {
		task->status &= ~TASK_STATUS_SUSPEND;
	}

	if (task->status & TASK_STATUS_PEND) {
		task->status &= ~TASK_STATUS_PEND;
		timeout_queue_del(&task->timeout);
	}

	if (task->status == TASK_STATUS_RUNNING) {
		if (current_task_get() == task) {
			sched_ready_queue_remove(task->cpu_id, task);
			task->status = TASK_STATUS_STOP;
			// notify service to delete
			code_unreachable();
		} else {
			// notify smp task pending schedule
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
	struct task *task = ID_TO_TASK(task_id);
	struct task *current_task = current_task_get();

	if (!task) {
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
	sched_ready_queue_add(task->cpu_id, task);
	task_locked_sched();
	sched_spin_unlock(key);

	return OK;
}

errno_t task_suspend(task_id_t task_id) {
	uint32_t key = 0;
	struct task *task = ID_TO_TASK(task_id);
	struct task *current_task = current_task_get();

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (is_in_irq()) {
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	key = sched_spin_lock();
	if (task->status & TASK_STATUS_SUSPEND) {
		sched_spin_unlock(key);
		return OK;
	}

	if (task->status == TASK_STATUS_STOP) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->status & TASK_STATUS_READY) {
		task->status &= ~TASK_STATUS_READY;
		sched_ready_queue_remove(task->cpu_id, task);
	}

	if (task->status & TASK_STATUS_RUNNING) {
		task->status &= ~TASK_STATUS_RUNNING;
		/* For no smp, delete task is OK */
		sched_ready_queue_remove(task->cpu_id, task);
	}

	task->status |= TASK_STATUS_SUSPEND;
	if (task == current_task) {
		task_locked_sched();
	}
	sched_spin_unlock(key);

	return OK;
}

errno_t task_suspend_self() { return task_suspend(task_self_id()); }

errno_t task_delay(uint64_t ticks) {
	struct task *task = current_task_get();
	uint32_t key = 0;

	if (is_in_irq()) {
		return ERRNO_TASK_IN_IRQ_STATUS;
	}

	if (task->flag & TASK_FLAG_SYSTEM) {
		return ERRNO_TASK_OPERATE_INVALID;
	}

	key = sched_spin_lock();
	if (TASK_SCHED_LOCKED(task)) {
		sched_spin_unlock(key);
		return ERRNO_TASK_IS_LOCKED;
	}

	if (task->status != TASK_STATUS_RUNNING) {
		sched_spin_unlock(key);
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (ticks > 0) {
		sched_ready_queue_remove(task->cpu_id, task);
		task->status = TASK_STATUS_PEND;
		task->timeout.deadline_ticks = current_ticks() + ticks;
		timeout_queue_add(&task->timeout);
	}

	task_locked_sched();
	sched_spin_unlock(key);

	return OK;
}

void task_lock() {
	uint32_t key = 0;
	struct task *task = NULL;

	key = arch_irq_save();
	task = current_task_get();
	task->lock_cnt++;
	arch_irq_restore(key);

	return;
}

void task_unlock() {
	uint32_t key = 0;
	struct task *task = NULL;

	key = arch_irq_save();
	task = current_task_get();

	if (task->lock_cnt > 0) {
		task->lock_cnt--;
		if (task->lock_cnt == 0) {
			arch_irq_restore(key);
			task_unlocked_sched();
			return;
		}
	}
	arch_irq_restore(key);

	return;
}
