#include <stdio.h>
#include <task.h>
#include <string.h>
#include <cpu.h>
#include <mutex.h>
#include <tick.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"
#define TEST_MUTEX_NAME "test_mutex"

static task_id_t test_task1_id = 0;
static task_id_t test_task2_id = 0;
static task_id_t test_task3_id = 0;
static mutex_id_t test_mutex_id = 0;

static void create_mutex() { mutex_create(TEST_MUTEX_NAME, &test_mutex_id); }

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint64_t i = 0;
	uint32_t cpu_id = arch_cpu_id_get();

	for (;;) {
		mutex_take(test_mutex_id, MUTEX_WAIT_FOREVER);
		printf("%s - cpu%u - %lu, enter\n", TEST_TASK1_NAME, cpu_id, i);
		mdelay(tick2ms(10));
		printf("%s - cpu%u - %lu, leave\n", TEST_TASK1_NAME, cpu_id, i++);
		mutex_give(test_mutex_id);
	}
}

static void create_test_task1() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK1_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task1_id, task_name, test_task1_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE,
					  TASK_FLAG_KERNEL | TASK_FLAG_INHERIT_PERM);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_priority_set(test_task1_id, 10 /* prioriy */);
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
	uint32_t cpu_id = arch_cpu_id_get();

	for (;;) {
		mutex_take(test_mutex_id, MUTEX_WAIT_FOREVER);
		printf("%s - cpu%u - %lu, enter\n", TEST_TASK2_NAME, cpu_id, i);
		mdelay(tick2ms(10));
		printf("%s - cpu%u - %lu, leave\n", TEST_TASK2_NAME, cpu_id, i++);
		mutex_give(test_mutex_id);
	}
}

static void create_test_task2() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK2_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task2_id, task_name, test_task2_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE,
					  TASK_FLAG_KERNEL | TASK_FLAG_INHERIT_PERM);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_priority_set(test_task2_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_cpu_affi_set(test_task2_id, 1);
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
	uint32_t cpu_id = arch_cpu_id_get();

	for (;;) {
		mutex_take(test_mutex_id, MUTEX_WAIT_FOREVER);
		printf("%s - cpu%u - %lu, enter\n", TEST_TASK3_NAME, cpu_id, i);
		mdelay(tick2ms(10));
		printf("%s - cpu%u - %lu, leave\n", TEST_TASK3_NAME, cpu_id, i++);
		mutex_give(test_mutex_id);
	}

	return;
}

static void create_test_task3() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK3_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task3_id, task_name, test_task3_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE,
					  TASK_FLAG_KERNEL | TASK_FLAG_INHERIT_PERM);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_priority_set(test_task3_id, 10 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_cpu_affi_set(test_task3_id, 2);
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

int main() {
	printf("enter root task\n");
	create_mutex();
	create_test_task1();
	create_test_task2();
	create_test_task3();

	return 0;
}
