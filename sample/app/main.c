#include <stdio.h>
#include <task.h>
#include <tick.h>
#include <string.h>
#include <task_sched.h>
#include <stack_trace.h>
#include <msgq.h>
#include <sem.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define TEST_TASK3_NAME "test_task3"
#define TEST_TASK4_NAME "test_task4"
#define TEST_TASK5_NAME "test_task5"
#define TEST_MSGQ_NAME "test_msgq"
#define TEST_MSGQ_NUM 5
#define TEST_MSGQ_SIZE 32
#define TEST_SEM_NAME "test_sem"
#define TEST_SEM_MAX_NUM 5

static task_id_t test_task1_id = 0;
static task_id_t test_task2_id = 0;
static task_id_t test_task3_id = 0;
static task_id_t test_task4_id = 0;
static task_id_t test_task5_id = 0;
static msgq_id_t test_msgq_id = 0;
static sem_id_t test_sem_id = 0;

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
		task_delay(20);
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
		task_delay(20);
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
	int32_t ret = 0;

	for (;;) {
		printf("task %s %d\n", TEST_TASK3_NAME, i++);
		task_suspend(test_task2_id);
		printf("task %s suspended\n", TEST_TASK2_NAME);
		task_stop(test_task1_id);
		printf("task %s stoped\n", TEST_TASK1_NAME);
		task_delay(20);
		task_resume(test_task2_id);
		printf("task %s resumed\n", TEST_TASK2_NAME);
		task_start(test_task1_id);
		printf("task %s started\n", TEST_TASK1_NAME);
		task_delay(20);
		ret = msgq_send(test_msgq_id, &i, sizeof(i), MSGQ_NO_WAIT);
		printf("msgq send: %u, ret = %d\n", i, ret);
		ret = sem_give(test_sem_id);
		printf("sem give: %u, ret = %d\n", i, ret);
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

static void test_task4_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	char msg[TEST_MSGQ_SIZE] = {0};
	uint32_t size = TEST_MSGQ_SIZE;
	int32_t ret = 0;

	for (;;) {
		size = TEST_MSGQ_SIZE;
		memset(msg, 0, TEST_MSGQ_SIZE);
		ret = msgq_receive(test_msgq_id, &msg, &size, MSGQ_WAIT_FOREVER);
		msg[TEST_MSGQ_SIZE - 1] = 0;
		printf("task %s received msg: %u ret = %d\n", TEST_TASK4_NAME,
			   *(uint32_t *)msg, ret);
	}
}

static void create_test_task4() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK4_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task4_id, task_name, test_task4_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(test_task4_id, 2 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task4_id);
	if (ret != OK) {
		printf("start task %s failed\n", task_name);
		return;
	}

	return;
}

static void test_task5_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;
	uint32_t i = 0;

	for (;;) {
		printf("task %s will take a semphore, i = %d\n", TEST_TASK5_NAME, i++);
		sem_take(test_sem_id, SEM_WAIT_FOREVER);
	}

	return;
}

static void create_test_task5() {
	errno_t ret = OK;
	char task_name[TASK_NAME_LEN] = {0};

	strncpy(task_name, TEST_TASK5_NAME, TASK_NAME_LEN);
	ret = task_create(&test_task5_id, task_name, test_task5_entry, NULL, NULL,
					  NULL, NULL, TASK_STACK_DEFAULT_SIZE, TASK_FLAG_USER);
	if (ret != OK) {
		printf("create task %s failed\n", task_name);
		return;
	}

	ret = task_prority_set(test_task5_id, 1 /* prioriy */);
	if (ret != OK) {
		printf("set task %s priority failed\n", task_name);
		return;
	}

	ret = task_start(test_task5_id);
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
	msgq_create(TEST_MSGQ_NAME, TEST_MSGQ_NUM, TEST_MSGQ_SIZE, &test_msgq_id);
	sem_create(TEST_SEM_NAME, 0, TEST_SEM_MAX_NUM, &test_sem_id);
	create_test_task1();
	create_test_task2();
	create_test_task3();
	create_test_task4();
	create_test_task5();
	task_suspend_self();
	code_unreachable();
}
