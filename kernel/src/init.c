#include <string.h>
#include <stdio.h>
#include <gic_v2.h>
#include <arch_timer.h>
#include <task_sched.h>
#include <cpu.h>
#include <stdio.h>

#define SHELL_LOGO "[LS Kernel]# "
#define shell_logo_show() printf(SHELL_LOGO)
extern void logo_show();

void root_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	uint64_t i = 0;
	for (;;) {
		printf("root task %d\n", i++);
		task_delay(1000);
	}
}

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
