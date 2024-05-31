#include <types.h>
#include <tick.h>
#include <irq.h>
#include <cpu.h>
#include <limits.h>
#include <arch_timer.h>
#include <menuconfig.h>

static uint64_t tick_counts[CONFIG_CPUS_MAX_NUM] = {0};

void tick_announce() {
	uint64_t key = arch_irq_save();
	tick_counts[arch_cpu_id_get()]++;
	arch_irq_restore(key);
}

uint64_t current_ticks() {
	uint64_t key = arch_irq_save();
	uint64_t ticks = tick_counts[arch_cpu_id_get()];
	arch_irq_restore(key);

	return ticks;
}

uint64_t current_cycles() {
	uint64_t cycles = 0;

	cycles = arch_timer_count();

	return cycles;
}

uint32_t ms2tick(uint32_t ms) {
	return (uint32_t)(((uint64_t)ms * CONFIG_TICK_PER_SECOND) / MS_PER_SECOND);
}

uint32_t tick2ms(uint32_t tick) {
	return (uint32_t)(((uint64_t)tick * MS_PER_SECOND) /
					  CONFIG_TICK_PER_SECOND);
}

uint32_t us2tick(uint32_t us) {
	return (uint32_t)(((uint64_t)us * CONFIG_TICK_PER_SECOND) / US_PER_SECOND);
}

uint32_t tick2us(uint32_t tick) {
	return (uint32_t)(((uint64_t)tick * US_PER_SECOND) /
					  CONFIG_TICK_PER_SECOND);
}

void udelay(uint32_t us) {
	uint64_t cycles = (uint64_t)us * arch_timer_freq_get() / US_PER_SECOND;
	uint64_t deadline = current_cycles() + cycles;

	while (current_cycles() < deadline) {
		__asm__ volatile("nop");
	}
}

void mdelay(uint32_t ms) {
	uint64_t cycles = (uint64_t)ms * arch_timer_freq_get() / MS_PER_SECOND;
	uint64_t deadline = current_cycles() + cycles;

	while (current_cycles() < deadline) {
		__asm__ volatile("nop");
	}
}
