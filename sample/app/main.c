#include <stdio.h>
#include <task.h>
#include <tick.h>
#include <string.h>
#include <task_sched.h>
#include <stack_trace.h>
#include <msgq.h>
#include <sem.h>
#include <mutex.h>
#include <timer.h>
#include <task.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"
#define TEST_TASK4_NAME "test_task4"
#define TEST_TASK5_NAME "test_task5"
#define TEST_TASK6_NAME "test_task6"
#define TEST_MSGQ_NAME "test_msgq"
#define TEST_MSGQ_NUM 5
#define TEST_MSGQ_SIZE 32
#define TEST_SEM_NAME "test_sem"
#define TEST_SEM_MAX_NUM 5
#define TEST_MUTEX_NAME "test_mutex"
#define TEST_TIMER_NAME "test_timer"

static task_id_t test_task1_id = 0;
static task_id_t test_task3_id = 0;
static task_id_t test_task6_id = 0;

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	uint32_t i = 0;

	for (;;) {
		arch_stack_default_walk(TEST_TASK1_NAME, current_task_get(), NULL);

		printf("task %s %d\n", TEST_TASK1_NAME, i++);
		task_delay(1);
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

	task_cpu_affi_set(test_task1_id, 1);

	ret = task_start(test_task1_id);
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
		printf("task %s i = %d\n", TEST_TASK3_NAME, i++);
		// task_stop(test_task1_id);
		// printf("task %s stoped\n", TEST_TASK1_NAME);
		task_delay(1);
		// task_start(test_task1_id);
		// printf("task %s started\n", TEST_TASK1_NAME);
		// task_delay(2);
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

	ret = task_prority_set(test_task3_id, 3 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	task_cpu_affi_set(test_task3_id, 2);

	ret = task_start(test_task3_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

static void test_task6_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint32_t i = 0;

	for (;;) {
		printf("task %s i = %d\n", TEST_TASK6_NAME, i++);
		task_delay(10);
	}

	return;
}

static void create_test_task6() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK6_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task6_id, task_name, test_task6_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	task_cpu_affi_set(test_task6_id, 3);

	ret = task_prority_set(test_task6_id, 6 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task6_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

void test_timeout_cb(void *arg) { (void)arg; }

void main_task_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	printf("enter root task\n");
	create_test_task1();
	create_test_task3();
	create_test_task6();
}
