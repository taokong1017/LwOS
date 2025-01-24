#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>
#include <task_sched.h>
#include <cpu.h>
#include <mem_mgr.h>
#include <cpu_ops_psci.h>
#include <smp.h>
#include <shell.h>

#define HEAP_SIZE (0x10000)
extern void logo_show();
extern struct shell shell_uart;
ALIGNED(16) char heap[HEAP_SIZE] = {0};

void kernel_start() {
	mem_init((void *)heap, HEAP_SIZE);
	logo_show();

	psci_init("hvc");
	arm_gic_init(true);
	arch_timer_init(true);
	percpu_init(0);
	main_task_create(0);
	shell_init(&shell_uart, NULL);
	smp_init();
	task_sched_start();
}
