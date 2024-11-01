#include <compiler.h>
#include <user_space.h>

#define TASK_STACK_SIZE 0x2000
static struct mem_domain app_mem_domain_a;
APP_PARTITION_DEFINE(domain_a);

#define ROOT_TASK_A_NAME "Root_Task_A"
APP_DATA(domain_a) task_id_t root_task_a_id = -1;
APP_DATA(domain_a) struct task root_task_a = {0};
APP_BSS(domain_a) char root_task_a_stack[TASK_STACK_SIZE] = {0};

#define TASK_A_NAME "Task_A"
APP_DATA(domain_a) task_id_t task_a_id = -1;
APP_DATA(domain_a) struct task task_a = {0};
APP_BSS(domain_a) char task_a_stack[TASK_STACK_SIZE] = {0};

static struct mem_domain app_mem_domain_b;
APP_PARTITION_DEFINE(domain_b);

#define ROOT_TASK_B_NAME "Root_Task_B"
APP_DATA(domain_b) task_id_t root_task_b_id = -1;
APP_DATA(domain_b) struct task root_task_b = {0};
APP_BSS(domain_b) char root_task_b_stack[TASK_STACK_SIZE] = {0};

#define TASK_B_NAME "Task_B"
APP_DATA(domain_b) task_id_t task_b_id = -1;
APP_DATA(domain_b) struct task task_b = {0};
APP_BSS(domain_b) char task_b_stack[TASK_STACK_SIZE] = {0};

static void task_a_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg2;

	forever() { user_task_delay(1); }
}

static void task_b_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	forever() { user_task_delay(1); }
}

static void user_root_task_a_entry(void *arg0, void *arg1, void *arg2,
								   void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	/* Create and start task A */
	user_task_create(&task_a_id, TASK_A_NAME, task_a_entry, NULL, NULL, NULL,
					 NULL, &task_a, &task_a_stack, TASK_STACK_SIZE,
					 TASK_FLAG_USER | TASK_FLAG_INHERIT_PERM);
	user_task_priority_set(task_a_id, TASK_PRIORITY_HIGHEST - 1);
	user_task_start(task_a_id);

	return;
}

static void user_root_task_b_entry(void *arg0, void *arg1, void *arg2,
								   void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	/* Create and start task B */
	user_task_create(&task_b_id, TASK_B_NAME, task_b_entry, NULL, NULL, NULL,
					 NULL, &task_b, &task_b_stack, TASK_STACK_SIZE,
					 TASK_FLAG_USER | TASK_FLAG_INHERIT_PERM);
	user_task_priority_set(task_b_id, TASK_PRIORITY_HIGHEST - 1);
	user_task_start(task_b_id);

	return;
}

int main() {
	app_mem_domain_init(&app_mem_domain_a, "domain_a", domain_a);
	mem_domain_set_up(&app_mem_domain_a);

	app_mem_domain_init(&app_mem_domain_b, "domain_b", domain_b);
	mem_domain_set_up(&app_mem_domain_b);

	/* 创建用户态根任务A */
	task_create_with_stack(&root_task_a_id, ROOT_TASK_A_NAME,
						   user_root_task_a_entry, NULL, NULL, NULL, NULL,
						   &root_task_a, &root_task_a_stack, TASK_STACK_SIZE,
						   TASK_FLAG_USER);
	task_mem_domain_add(root_task_a_id, &app_mem_domain_a);
	task_priority_set(root_task_a_id, TASK_PRIORITY_HIGHEST);
	task_start(root_task_a_id);

	/* 创建用户态根任务B */
	task_create_with_stack(&root_task_b_id, ROOT_TASK_B_NAME,
						   user_root_task_b_entry, NULL, NULL, NULL, NULL,
						   &root_task_b, &root_task_b_stack, TASK_STACK_SIZE,
						   TASK_FLAG_USER);
	task_mem_domain_add(root_task_b_id, &app_mem_domain_b);
	task_priority_set(root_task_b_id, TASK_PRIORITY_HIGHEST);
	task_start(root_task_b_id);

	return 0;
}
