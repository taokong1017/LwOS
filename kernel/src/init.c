#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>
#include <task_sched.h>
#include <cpu.h>
#include <mem_mgr.h>
#include <cpu_ops_psci.h>
#include <smp.h>

#define SHELL_LOGO "[LW shell] > "
#define shell_logo_show() printf(SHELL_LOGO)
#define HEAP_SIZE (0x10000000)
extern void logo_show();
ALIGNED(16) char heap[HEAP_SIZE] = {0};

void kernel_start() {
	mem_init((void *)heap, HEAP_SIZE);
	logo_show();
	shell_logo_show();

	psci_init("hvc");
	arm_gic_init(true);
	arch_timer_init(true);
	percpu_init(0);
	main_task_create(0);
	smp_init();
	task_sched_start();
}
