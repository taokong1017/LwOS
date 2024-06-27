#include <stdio.h>
#include <task.h>
#include <tick.h>
#include <string.h>
#include <task_sched.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"

static bool show_Linker(void *cookie, phys_addr_t pc) {
	uint32_t *level = (uint32_t *)cookie;
	printf("%u: 0x%016llx\n", (*level)++, pc);
	return true;
}

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	uint32_t i = 0;
	uint32_t level = 0;
	uint32_t prioriy = -1;

	for (;;) {
		task_prority_get(task_self_id(), &prioriy);
		printf("task %s priority %d\n", TEST_TASK1_NAME, prioriy);

		level = 0;
		arch_stack_walk(show_Linker, &level, current_task_get(), NULL);

		printf("task %s %d\n", TEST_TASK1_NAME, i++);
		task_delay(100);
	}
}

static void create_test_task1() {
	task_id_t task_id = 0;
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK1_NAME, TASK_NAME_LEN);
	ret = task_create(&task_id, task_name, test_task1_entry, NULL, NULL, NULL,
					  NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(task_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(task_id);
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

	for (;;) {
		printf("task %s %d\n", TEST_TASK2_NAME, i++);
		task_delay(500);
	}
}

static void create_test_task2() {
	task_id_t task_id = 0;
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK2_NAME, TASK_NAME_LEN);
	ret = task_create(&task_id, task_name, test_task2_entry, NULL, NULL, NULL,
					  NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(task_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(task_id);
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
		task_delay(500);
	}
}

static void create_test_task3() {
	task_id_t task_id = 0;
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK3_NAME, TASK_NAME_LEN);
	ret = task_create(&task_id, task_name, test_task3_entry, NULL, NULL, NULL,
					  NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(task_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(task_id);
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
	uint64_t i = 0;

	create_test_task1();
	create_test_task2();
	create_test_task3();
	for (;;) {
		printf("task main %d\n", i++);
		task_delay(1000);
	}
}
