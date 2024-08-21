#include <spin_lock.h>
#include <task.h>
#include <arch_spinlock.h>
#include <irq.h>
#include <menuconfig.h>
#include <task_sched.h>
#include <log.h>
#include <arch_regs.h>
#include <stack_trace.h>
#include <string.h>
#include <cpu.h>

#define SPIN_LOCK_TAG "SPIN_LOCK"
#define default_str_fill(str) ((str == NULL) ? "unkown" : str)
#define STACK_WALK_SIZE 20

struct spinlock_trace_info {
	uint32_t total_level;
	uint32_t cpu_id;
	virt_addr_t trace[STACK_WALK_SIZE];
};

struct spinlock_trace_info spin_lock_trace_info;

static bool save_Linker(void *cookie, virt_addr_t pc) {
	struct spinlock_trace_info *info = (struct spinlock_trace_info *)cookie;

	if (info->total_level < STACK_WALK_SIZE) {
		info->trace[info->total_level++] = pc;
		return true;
	}

	return false;
}

static void spin_lock_trace() {
	spin_lock_trace_info.total_level = 0;
	struct task *task = current_task_get();

	spin_lock_trace_info.cpu_id = arch_cpu_id_get();
	memset(spin_lock_trace_info.trace, 0, sizeof(spin_lock_trace_info.trace));
	if (task) {
		arch_stack_walk(save_Linker, &spin_lock_trace_info, NULL, NULL);
	} else {
		arch_stack_walk(save_Linker, &spin_lock_trace_info, task, NULL);
	}

	return;
}

void spin_lock_trace_dump() {
	uint32_t i = 0;
	struct spinlock_trace_info *info = &spin_lock_trace_info;

	printf("[%s - %d] trace info: \n", SPIN_LOCK_TAG, info->cpu_id);
	for (i = 0; i < info->total_level; i++) {
		printf("  %u: 0x%016llx\n", i, info->trace[i]);
	}
}

void spin_lock(struct spinlock *lock) {
	task_lock();
	arch_spin_lock(&lock->rawlock);
	spin_lock_trace();
	log_debug(SPIN_LOCK_TAG, "spin lock %s is owned by %s\n",
			 default_str_fill(lock->name), current_task_get()->name);

	return;
}

int32_t spin_trylock(struct spinlock *lock) {
	int32_t ret = 0;

	task_lock();
	ret = arch_spin_try_lock(&lock->rawlock);
	if (ret != OK) {
		task_unlock();
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}

	return OK;
}

void spin_unlock(struct spinlock *lock) {
	arch_spin_unlock(&lock->rawlock);
	task_unlock();
	spin_lock_trace_info.total_level = 0;
	log_debug(SPIN_LOCK_TAG, "spin lock %s is free\n",
			  default_str_fill(lock->name));

	return;
}

void spin_lock_save(struct spinlock *lock, uint32_t *key) {
	*key = arch_irq_save();
	spin_lock(lock);

	return;
}

void spin_lock_restore(struct spinlock *lock, uint32_t key) {
	spin_unlock(lock);
	arch_irq_restore(key);

	return;
}

void spin_lock_init(struct spinlock *lock) { lock->rawlock = 0; }

bool spin_lock_is_locked(struct spinlock *lock) { return lock->rawlock != 0; }
