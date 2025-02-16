#include <compiler.h>
#include <user_space.h>
#include <stdlib.h>

#define TASK_STACK_SIZE 0x2000
user_mem_domain_define();

#define ROOT_TASK_NAME "Root_Task"
user_data_section task_id_t root_task_id = -1;
user_data_section struct task root_task = {0};
user_bss_section char root_stack[TASK_STACK_SIZE] = {0};

static void user_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	while (1) {
		user_task_delay(1);
	}
}

int main() {
	user_mem_domain_init();

	/* 创建用户态根任务 */
	task_create_with_stack(&root_task_id, ROOT_TASK_NAME, user_task_entry, NULL,
						   NULL, NULL, NULL, &root_task, &root_stack,
						   TASK_STACK_SIZE, TASK_FLAG_USER);
	task_mem_domain_add(root_task_id, &user_mem_domain);
	task_priority_set(root_task_id, TASK_PRIORITY_HIGHEST);
	task_start(root_task_id);

	return 0;
}
