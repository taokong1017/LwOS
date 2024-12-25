#include <types.h>
#include <tick.h>
#include <irq.h>
#include <cpu.h>
#include <limits.h>
#include <arch_timer.h>
#include <timeout.h>
#include <menuconfig.h>
#include <log.h>
#include <spin_lock.h>
#include <task_sched.h>

#define TICK_TAG "TICK"
uint64_t tick_counts[CONFIG_CPUS_MAX_NUM] = {0};

void tick_announce() {
	uint32_t cpu_id = arch_cpu_id_get();

	tick_counts[cpu_id]++;

	log_debug(TICK_TAG, "cpu%u tick: %llu\n", cpu_id, tick_counts[cpu_id]);

	if (!sched_spin_is_locked()) {
		timeout_queue_handle(tick_counts[cpu_id]);
	}
}

uint64_t current_ticks_get() {
	uint64_t tick = 0;

	tick = tick_counts[arch_cpu_id_get()];

	return tick;
}

uint64_t current_cycles_get() {
	uint64_t cycles = 0;

	cycles = arch_timer_count();

	return cycles;
}

uint32_t ms2tick(uint32_t ms) {
	return (uint32_t)(((uint64_t)ms * CONFIG_TICKS_PER_SECOND) / MS_PER_SECOND);
}

uint32_t tick2ms(uint32_t tick) {
	return (uint32_t)(((uint64_t)tick * MS_PER_SECOND) /
					  CONFIG_TICKS_PER_SECOND);
}

uint32_t us2tick(uint32_t us) {
	return (uint32_t)(((uint64_t)us * CONFIG_TICKS_PER_SECOND) / US_PER_SECOND);
}

uint32_t tick2us(uint32_t tick) {
	return (uint32_t)(((uint64_t)tick * US_PER_SECOND) /
					  CONFIG_TICKS_PER_SECOND);
}

void udelay(uint32_t us) {
	uint64_t cycles = (uint64_t)us * arch_timer_freq_get() / US_PER_SECOND;
	uint64_t deadline = current_cycles_get() + cycles;

	while (current_cycles_get() < deadline) {
		__asm__ volatile("nop");
	}
}

void mdelay(uint32_t ms) {
	uint64_t cycles = (uint64_t)ms * arch_timer_freq_get() / MS_PER_SECOND;
	uint64_t deadline = current_cycles_get() + cycles;

	while (current_cycles_get() < deadline) {
		__asm__ volatile("nop");
	}
}
