#include <stdio.h>
#include <task.h>
#include <tick.h>
#include <string.h>
#include <task_sched.h>
#include <cpu.h>
#include <msgq.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"
#define TEST_MSGQ1_NAME "test_msgq1"
#define TEST_MSGQ2_NAME "test_msgq2"
#define TEST_MSGQ_NUM 1
#define TEST_MSGQ_SIZE 32
#define TEST_MSGQ_MSG "test"

static task_id_t test_task1_id = 0;
static task_id_t test_task2_id = 0;
static task_id_t test_task3_id = 0;
static msgq_id_t test_msgq1_id = 0;
static msgq_id_t test_msgq2_id = 0;

static void create_msgqs() {
	msgq_create(TEST_MSGQ1_NAME, TEST_MSGQ_NUM, TEST_MSGQ_SIZE, &test_msgq1_id);
	msgq_create(TEST_MSGQ2_NAME, TEST_MSGQ_NUM, TEST_MSGQ_SIZE, &test_msgq2_id);
}

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint64_t i = 0;
	uint64_t data = 0;
	uint32_t size = 0;

	for (;;) {
		size = sizeof(data);
		msgq_receive(test_msgq1_id, &data, &size, MSGQ_WAIT_FOREVER);
		printf("%s - cpu%u - %lu: %lu\n", TEST_TASK1_NAME, arch_cpu_id_get(),
			   i++, data);
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

	ret = task_cpu_affi_set(test_task1_id, 1);
	if (ret != OK) {
		printf("set task %s cpu affinity failed\n", task_name);
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
	uint64_t i = 0;
	uint64_t data = 0;
	uint32_t size = 0;

	for (;;) {
		size = sizeof(data);
		msgq_receive(test_msgq2_id, &data, &size, MSGQ_WAIT_FOREVER);
		printf("%s - cpu%u - %lu: %lu\n", TEST_TASK2_NAME, arch_cpu_id_get(),
			   i++, data);
		task_delay(10);
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

	ret = task_cpu_affi_set(test_task2_id, 2);
	if (ret != OK) {
		printf("set task %s cpu affinity failed\n", task_name);
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

	uint64_t i = 0;
	uint32_t ticks = 10;
	uint64_t value = 10;

	for (;;) {
		printf("%s - cpu%u - %lu: %lu\n", TEST_TASK3_NAME, arch_cpu_id_get(),
			   i++, value);
		msgq_send(test_msgq1_id, &value, sizeof(value), MSGQ_WAIT_FOREVER);
		msgq_send(test_msgq2_id, &value, sizeof(value), MSGQ_WAIT_FOREVER);
		value++;
		task_delay(ticks);
		printf("%s has delayed %u ticks\n", TEST_TASK3_NAME, ticks);
	}

	return;
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

	ret = task_prority_set(test_task3_id, 9 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_cpu_affi_set(test_task3_id, 1);
	if (ret != OK) {
		printf("set task %s cpu affinity failed\n", task_name);
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
	create_msgqs();
	create_test_task1();
	create_test_task2();
	create_test_task3();
}
