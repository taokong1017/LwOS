#include <user_space.h>

#define TASK_NAME "Task_006_1"
#define TASK_STACK_SIZE 0x8000
static struct mem_domain app_mem_domain;
APP_PARTITION_DEFINE(app_1);
APP_DATA(app_1) task_id_t task_id = -1;
APP_DATA(app_1) struct task task = {0};
APP_BSS(app_1) char stack[TASK_STACK_SIZE] = {0};

static void task1(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg0;
	(void)arg0;
	(void)arg0;

	forever();
}

int main() {
	mem_domain_init(&app_mem_domain, "Task_006_Domain");
	mem_domain_kernel_ranges_copy(&app_mem_domain);
	mem_domain_set_up(&app_mem_domain);
	user_task_create(&task_id, TASK_NAME, task1, NULL, NULL, NULL, NULL, &task,
					 &stack, TASK_STACK_SIZE, TASK_FLAG_USER);
	user_task_start(task_id);

	return 0;
}
