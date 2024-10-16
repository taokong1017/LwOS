#include <user_space.h>

#define TASK_NAME "Task_006_1"
#define TASK_STACK_SIZE 0x8000
static struct mem_domain app_mem_domain;
APP_PARTITION_DEFINE(part1);
APP_DATA(part1) task_id_t task_id = -1;
APP_DATA(part1) struct task task = {0};
APP_BSS(part1) char stack[TASK_STACK_SIZE] = {0};

static void part_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg0;
	(void)arg0;
	(void)arg0;

	forever();
}

int main() {
	app_mem_domain_init(&app_mem_domain, "part1", part1);
	mem_domain_set_up(&app_mem_domain);

	/* 创建第一个分区任务 */
	task_create_with_stack(&task_id, TASK_NAME, part_task_entry, NULL, NULL, NULL, NULL,
						   &task, &stack, TASK_STACK_SIZE, TASK_FLAG_USER);
	task_start(task_id);

	return 0;
}
