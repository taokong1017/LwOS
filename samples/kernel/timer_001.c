#include <stdio.h>
#include <task.h>
#include <string.h>
#include <timer.h>
#include <cpu.h>

#define TEST_TASK1_NAME "test_task1"
#define TIMER_ONESHOT_NAME "timer_oneshot"
#define TIMER_PERIODIC_NAME "timer_periodic"
#define TIMER_PERIOD 10

static task_id_t test_task1_id = 0;
static timer_id_t timer_oneshot_id = 0;
static timer_id_t timer_periodic_id = 0;
static uint64_t timer_oneshot_count = 0;
static uint64_t timer_periodic_count = 0;

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	for (;;) {
		task_delay(10);
		timer_start(timer_oneshot_id);
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

	ret = task_cpu_affi_set(test_task1_id, 2);
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

static void timer_oneshot_cb(void *arg) {
	uint64_t *timer_count = (uint64_t *)arg;

	(*timer_count)++;
	printf("oneshot - cpu%u: %d\n", arch_cpu_id_get(), *timer_count);
}

static void timer_periodic_cb(void *arg) {
	uint64_t *timer_count = (uint64_t *)arg;

	(*timer_count)++;
	printf("periodicshot - cpu%u: %d\n", arch_cpu_id_get(), *timer_count);
}

static void create_timers() {
	timer_create(TIMER_ONESHOT_NAME, TIMER_TYPE_ONE_SHOT, TIMER_PERIOD,
				 timer_oneshot_cb, &timer_oneshot_count, &timer_oneshot_id);
	timer_create(TIMER_PERIODIC_NAME, TIMER_TYPE_PERIODIC, TIMER_PERIOD,
				 timer_periodic_cb, &timer_periodic_count, &timer_periodic_id);
	timer_start(timer_oneshot_id);
	timer_start(timer_periodic_id);
}

void main(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	printf("enter root task\n");
	create_timers();
	create_test_task1();
}
