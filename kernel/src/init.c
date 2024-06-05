#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>

#define SHELL_LOGO "[LS Kernel]# "
#define shell_logo_show() printf(SHELL_LOGO)
extern void logo_show();
extern void arch_timer_compare_isr(const void *arg);
extern void idle_task_create();

void kernel_start() {
	logo_show();
	shell_logo_show();

	arm_gic_init();
	arch_timer_init();
	idle_task_create();
	while (1) {
		/* TO DO */;
	}
}
