#include <stdio.h>
#include <task.h>
#include <tick.h>
#include <string.h>
#include <task_sched.h>
#include <stack_trace.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"

static task_id_t test_task1_id = 0;
static task_id_t test_task2_id = 0;
static task_id_t test_task3_id = 0;

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	uint32_t i = 0;
	uint32_t prioriy = -1;

	for (;;) {
		task_prority_get(task_self_id(), &prioriy);
		printf("task %s priority %d\n", TEST_TASK1_NAME, prioriy);

		arch_stack_default_walk(TEST_TASK1_NAME, current_task_get(), NULL);
		arch_stack_default_walk(TEST_TASK2_NAME, (struct task *)test_task2_id,
								NULL);

		printf("task %s %d\n", TEST_TASK1_NAME, i++);
		task_delay(500);
	}
}

static void create_test_task1() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK1_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task1_id, task_name, test_task1_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(test_task1_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task1_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

static void test_task2_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint32_t i = 0;
	errno_t ret = 0;
	uint32_t priority = 1;
	uint32_t priority1 = 15;

	for (;;) {
		printf("task %s %d\n", TEST_TASK2_NAME, i++);
		task_delay(1000);
		ret = task_prority_set(test_task1_id, priority);
		if (ret == OK) {
			printf("set task %s priority %d\n", TEST_TASK1_NAME, priority);
		} else {
			printf("set task %s priority %d failed\n", TEST_TASK1_NAME,
				   priority);
		}
		ret = task_prority_set(test_task2_id, priority1);
		if (ret == OK) {
			printf("set task %s priority %d\n", TEST_TASK2_NAME, priority1);
		} else {
			printf("set task %s priority %d failed\n", TEST_TASK2_NAME,
				   priority1);
		}
	}
}

static void create_test_task2() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK2_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task2_id, task_name, test_task2_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(test_task2_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task2_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

static void test_task3_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	uint32_t i = 0;

	for (;;) {
		printf("task %s %d\n", TEST_TASK3_NAME, i++);
		task_suspend(test_task2_id);
		printf("task %s suspend\n", TEST_TASK2_NAME);
		task_delay(2000);
		task_resume(test_task2_id);
		printf("task %s resume\n", TEST_TASK2_NAME);
		task_delay(100);
	}
}

static void create_test_task3() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK3_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task3_id, task_name, test_task3_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(test_task3_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task3_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

void main_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	printf("enter root task\n");
	create_test_task1();
	create_test_task2();
	create_test_task3();
	task_suspend_self();
	code_unreachable();
}
