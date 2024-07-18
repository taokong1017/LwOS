#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>
#include <task_sched.h>
#include <cpu.h>
#include <mem.h>
#include <cpu_ops_psci.h>

#define SHELL_LOGO "[LS Kernel]# "
#define shell_logo_show() printf(SHELL_LOGO)
#define HEAP_SIZE (0x10000000)
extern void logo_show();
ALIGNED(16) char heap[HEAP_SIZE] = {0};

void kernel_start() {
	mem_init((void *)heap, HEAP_SIZE);
	logo_show();
	shell_logo_show();

	psci_init("hvc");
	arm_gic_init();
	arch_timer_init();
	percpu_init(0);
	task_sched_init();
	code_unreachable();
}
