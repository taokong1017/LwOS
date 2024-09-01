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
#define OWNER_SIZE 32
#define IRQ_OWNER "IRQ"

struct spinlock_trace_info {
	uint32_t total_level;
	uint32_t cpu_id;
	virt_addr_t trace[STACK_WALK_SIZE];
	char lock_name[OWNER_SIZE];
	char owner[OWNER_SIZE];
};

struct spinlock_trace_info spin_lock_trace_info = {.owner = "unkown"};

static bool save_Linker(void *cookie, virt_addr_t pc) {
	struct spinlock_trace_info *info = (struct spinlock_trace_info *)cookie;

	if (info->total_level < STACK_WALK_SIZE) {
		info->trace[info->total_level++] = pc;
		return true;
	}

	return false;
}

static void spin_lock_trace(struct spinlock *lock) {
	spin_lock_trace_info.total_level = 0;
	struct task *task = current_task_get();

	spin_lock_trace_info.cpu_id = arch_cpu_id_get();
	memset(spin_lock_trace_info.trace, 0, sizeof(spin_lock_trace_info.trace));
	strncpy(spin_lock_trace_info.lock_name, lock->name, OWNER_SIZE);
	if (is_in_irq()) {
		arch_stack_walk(save_Linker, &spin_lock_trace_info, NULL, NULL);
		strncpy(spin_lock_trace_info.owner, IRQ_OWNER, OWNER_SIZE);
	} else if (task) {
		arch_stack_walk(save_Linker, &spin_lock_trace_info, task, NULL);
		strncpy(spin_lock_trace_info.owner, task->name, OWNER_SIZE);
	}

	return;
}

void spin_lock_trace_dump() {
	uint32_t i = 0;
	struct spinlock_trace_info *info = &spin_lock_trace_info;

	printf("[%s - CPU%d - %s]\nTrace Info: \n", info->lock_name, info->cpu_id,
		   info->owner);
	for (i = 0; i < info->total_level; i++) {
		printf("  %u: 0x%016llx\n", i, info->trace[i]);
	}
}

void spin_lock(struct spinlock *lock) {
	arch_spin_lock(&lock->rawlock);
	spin_lock_trace(lock);
	log_debug(SPIN_LOCK_TAG, "spin lock %s is owned by %s\n",
			  default_str_fill(lock->name), current_task_get()->name);

	return;
}

int32_t spin_trylock(struct spinlock *lock) {
	int32_t ret = 0;

	ret = arch_spin_try_lock(&lock->rawlock);
	if (ret != OK) {
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}

	return OK;
}

void spin_unlock(struct spinlock *lock) {
	arch_spin_unlock(&lock->rawlock);
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
