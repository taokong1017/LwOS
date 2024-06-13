#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>
#include <task_sched.h>
#include <cpu.h>

#define SHELL_LOGO "[LS Kernel]# "
#define shell_logo_show() printf(SHELL_LOGO)
extern void logo_show();
extern void arch_timer_compare_isr(const void *arg);

void kernel_start() {
	logo_show();
	shell_logo_show();

	arm_gic_init();
	arch_timer_init();
	task_sched_init();
	while (1) {
		/* TO DO */;
	}
}
