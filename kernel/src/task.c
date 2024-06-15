#include <types.h>
#include <task.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <arch_task.h>
#include <task_sched.h>

#define ALIGN(start, align) ((start + align - 1) & ~(align - 1))
#define TASK_TO_ID(task) ((task_id_t)task)

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

	if (stack_size < TASK_STACK_SIZE_MIN) {
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
	task->entry(task->args[0], task->args[1], task->args[2], task->args[3]);
}

errno_t task_prority_set(task_id_t task_id, uint32_t prioriy) {
	struct task *task = ID_TO_TASK(task_id);

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (prioriy < TASK_PRIORITY_LOWEST || prioriy > TASK_PRIORITY_HIGHEST) {
		return ERRNO_TASK_PRIOR_ERROR;
	}

	do_sched_spin_lock() {
		if (is_in_irq()) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_IN_IRQ_STATUS;
		}
		if (task->status == TASK_STATUS_READY) {
			sched_ready_queue_remove(task->cpu_id, task);
			task->priority = prioriy;
			sched_ready_queue_add(task->cpu_id, task);
			task_resched();
		} else {
			task->priority = prioriy;
		}
	}

	return OK;
}

errno_t task_prority_get(task_id_t task_id, uint32_t *prioriy) {
	struct task *task = ID_TO_TASK(task_id);

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (!prioriy) {
		return ERRNO_TASK_PRIORITY_EMPTY;
	}

	do_sched_spin_lock() {
		if (is_in_irq()) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_IN_IRQ_STATUS;
		}
		*prioriy = task->priority;
	}

	return OK;
}

errno_t task_start(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (task->status != TASK_STATUS_STOP) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	do_sched_spin_lock() {
		if (is_in_irq()) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_IN_IRQ_STATUS;
		}
		task->status = TASK_STATUS_READY;
		sched_ready_queue_add(task->cpu_id, task);
		task_resched();
	}

	return OK;
}

static task_id_t task_self_id() {
	task_id_t task_id = 0;

	do_sched_spin_lock() {
		if (is_in_irq()) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_IN_IRQ_STATUS;
		}
		task_id = current_task_get()->id;
	}

	return task_id;
}

errno_t task_stop(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	uint32_t temp_status = 0;

	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	do_sched_spin_lock() {
		if (is_in_irq()) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_IN_IRQ_STATUS;
		}

		temp_status = task->status;
		if (temp_status == TASK_STATUS_STOP) {
			sched_spin_unlock(lock_key);
			return ERRNO_TASK_STATUS_INVALID;
		}

		if (temp_status == TASK_STATUS_READY) {
			sched_ready_queue_remove(task->cpu_id, task);
		}

		if (temp_status & TASK_STATUS_SUSPEND) {
			task->status &= ~TASK_STATUS_SUSPEND;
			// TO DO
		}

		if (temp_status & TASK_STATUS_PEND) {
			task->status &= ~TASK_STATUS_PEND;
			// TO DO
		}

		task->status = TASK_STATUS_STOP;
		if (temp_status == TASK_STATUS_RUNNING) {
			if (current_task_get()->id == task_id) {
				sched_ready_queue_remove(task->cpu_id, task);
				task_resched();
			} else {
				// send task->cpu_id resched IPI
			}
		}
	}

	return OK;
}

errno_t task_stop_self() { return task_stop(task_self_id()); }

errno_t task_resume(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (!(task->status & TASK_STATUS_SUSPEND)) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	task->status &= ~TASK_STATUS_SUSPEND;
	// TO DO

	return OK;
}

errno_t task_suspend(task_id_t task_id) {
	struct task *task = ID_TO_TASK(task_id);
	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (task->status & TASK_STATUS_SUSPEND) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->status == TASK_STATUS_STOP) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	task->status &= ~TASK_STATUS_READY;
	task->status &= ~TASK_STATUS_RUNNING;
	task->status |= TASK_STATUS_SUSPEND;
	// TO DO

	return OK;
}

errno_t task_suspend_self() { return task_suspend(task_self_id()); }

errno_t task_delay(task_id_t task_id, task_id_t tick) {
	struct task *task = ID_TO_TASK(task_id);
	if (!task) {
		return ERRNO_TASK_ID_INVALID;
	}

	if (task->status == TASK_STATUS_STOP) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	if (task->status & TASK_STATUS_PEND) {
		return ERRNO_TASK_STATUS_INVALID;
	}

	task->status &= ~TASK_STATUS_READY;
	task->status &= ~TASK_STATUS_RUNNING;
	task->status |= TASK_STATUS_PEND;
	// TO DO

	return OK;
}

errno_t task_delay_self(task_id_t tick) {
	return task_delay(task_self_id(), tick);
}
