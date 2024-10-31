#include <compiler.h>
#include <user_space.h>
#include <tick.h>

#define TASK_STACK_SIZE 0x2000
static struct mem_domain app_mem_domain;
APP_PARTITION_DEFINE(app1);

#define ROOT_TASK_NAME "Root_Task"
APP_DATA(app1) task_id_t root_task_id = -1;
APP_DATA(app1) struct task root_task = {0};
APP_BSS(app1) char root_stack[TASK_STACK_SIZE] = {0};

#define TASK_A_NAME "Task_A"
APP_DATA(app1) task_id_t task_a_id = -1;
APP_DATA(app1) struct task task_a = {0};
APP_BSS(app1) char task_a_stack[TASK_STACK_SIZE] = {0};

#define TASK_B_NAME "Task_B"
APP_DATA(app1) task_id_t task_b_id = -1;
APP_DATA(app1) struct task task_b = {0};
APP_BSS(app1) char task_b_stack[TASK_STACK_SIZE] = {0};

#define USER_MUTEX_NAME "User_Mutex"
APP_DATA(app1) mutex_id_t user_mutex_id = -1;

static void my_delay(int32_t i) {
	while (i--)
		;
}

static void task_a_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	mutex_id_t mutex_id = (sem_id_t)arg0;

	forever() {
		user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
		my_delay(1000);
		user_mutex_give(mutex_id);
		user_task_delay(3);
	}
}

static void task_b_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg1;
	(void)arg2;
	(void)arg3;
	mutex_id_t mutex_id = (sem_id_t)arg0;

	forever() {
		user_mutex_take(mutex_id, MUTEX_WAIT_FOREVER);
		my_delay(33);
		user_mutex_give(mutex_id);
		user_task_delay(2);
	}
}

static void user_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg0;
	(void)arg0;
	(void)arg0;

	/* Create user mutex */
	user_mutex_create(USER_MUTEX_NAME, &user_mutex_id);

	/* Create and start task A */
	user_task_create(&task_a_id, TASK_A_NAME, task_a_entry,
					 (void *)user_mutex_id, NULL, NULL, NULL, &task_a,
					 &task_a_stack, TASK_STACK_SIZE,
					 TASK_FLAG_USER | TASK_FLAG_INHERIT_PERM);
	user_task_priority_set(task_a_id, TASK_PRIORITY_HIGHEST - 1);
	user_task_start(task_a_id);

	/* Create and start task B */
	user_task_create(&task_b_id, TASK_B_NAME, task_b_entry,
					 (void *)user_mutex_id, NULL, NULL, NULL, &task_b,
					 &task_b_stack, TASK_STACK_SIZE,
					 TASK_FLAG_USER | TASK_FLAG_INHERIT_PERM);
	user_task_priority_set(task_b_id, TASK_PRIORITY_HIGHEST - 1);
	user_task_start(task_b_id);

	return;
}

int main() {
	app_mem_domain_init(&app_mem_domain, "app1", app1);
	mem_domain_set_up(&app_mem_domain);

	/* 创建用户态根任务 */
	task_create_with_stack(&root_task_id, ROOT_TASK_NAME, user_task_entry, NULL,
						   NULL, NULL, NULL, &root_task, &root_stack,
						   TASK_STACK_SIZE, TASK_FLAG_USER);
	task_mem_domain_add(root_task_id, &app_mem_domain);
	task_priority_set(root_task_id, TASK_PRIORITY_HIGHEST);
	task_start(root_task_id);

	return 0;
}
