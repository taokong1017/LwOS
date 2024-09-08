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
#define IRQ_OWNER "IRQ"
#define default_str_fill(str) ((str == NULL) ? "unkown" : str)

static bool save_Linker(void *cookie, virt_addr_t pc) {
	struct spinlock *lock = (struct spinlock *)cookie;

	if (lock->level < STACK_WALK_SIZE) {
		lock->trace[lock->level++] = pc;
		return true;
	}

	return false;
}

static void spin_lock_trace(struct spinlock *lock) {
	struct task *task = current_task_get();

	lock->cpu_id = arch_cpu_id_get();
	lock->daif = arch_irq_status();
	if (is_in_irq()) {
		arch_stack_walk(save_Linker, lock, NULL, NULL);
		strncpy(lock->owner, IRQ_OWNER, OWNER_NAME_LEN);
	} else if (task) {
		arch_stack_walk(save_Linker, lock, task, NULL);
		strncpy(lock->owner, task->name, OWNER_NAME_LEN);
	}

	return;
}

static void spin_lock_trace_free(struct spinlock *lock) {
	lock->level = 0;
	lock->cpu_id = -1;
	lock->daif = -1;
	memset(lock->owner, 0, OWNER_NAME_LEN);
	memset(lock->trace, 0, sizeof(lock->trace));
}

void spin_lock_dump(struct spinlock *lock) {
	uint32_t i = 0;

	if ((strlen(lock->owner) == 0) && (lock->level == 0)) {
		printf("NO Spin Lock trace\n");
		return;
	}

	printf("[Trace Info]:\n");
	printf("Name:\t%s\n", lock->name);
	printf("CPU:\tcpu%u\n", lock->cpu_id);
	printf("DAIF:\t0x%x\n", lock->daif);
	printf("Owner:\t%s\n", lock->owner);
	printf("Trace Info:\n");
	for (i = 0; i < lock->level; i++) {
		printf("  %u: 0x%016llx\n", i, lock->trace[i]);
	}
}

void spin_lock(struct spinlock *lock) {
	arch_spin_lock(&lock->rawlock);
	spin_lock_trace(lock);
	log_debug(SPIN_LOCK_TAG, "spin lock %s is owned by %s\n", lock->name,
			  current_task_get()->name);

	return;
}

int32_t spin_trylock(struct spinlock *lock) {
	int32_t ret = 0;

	ret = arch_spin_try_lock(&lock->rawlock);
	if (ret != OK) {
		return ERRNO_SPINLOCK_TRY_LOCK_FAILED;
	}
	spin_lock_trace(lock);

	return OK;
}

void spin_unlock(struct spinlock *lock) {
	arch_spin_unlock(&lock->rawlock);
	spin_lock_trace_free(lock);
	log_debug(SPIN_LOCK_TAG, "spin lock %s is free\n", lock->name);

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
