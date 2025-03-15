#include <stdio.h>
#include <string.h>
#include <task_sched.h>
#include <task.h>
#include <rpmsg.h>
#include <stdlib.h>
#include <smp.h>
#include <gic_v2.h>
#include <irq.h>

#define TEST_TASK1_NAME "test_task1"
#define TEST_TASK2_NAME "test_task2"
#define BUFFER_SIZE 4096
#define virt_to_phys(addr) ((phys_addr_t)(addr))

static task_id_t test_task1_id = 0;
static task_id_t test_task2_id = 0;
static char buffer[BUFFER_SIZE] = {0};
static struct rpmsg_buf_info buf_info = {
	.h2r_buf_num = 4,
	.h2r_buf_size = 100,
	.r2h_buf_num = 4,
	.r2h_buf_size = 100,
};

static int rpmsg_msg_callback(void *data, uint32_t len) {
	int new_data = *(int *)data;

	printf("callback data 0x%x\n", new_data);
	return len;
}

static void test_task1_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	struct shm_mem_info shm_mem = {
		.virt = (void *)buffer,
		.phys = virt_to_phys(buffer),
		.size = BUFFER_SIZE,
		.sync_type = SHM_SYNC_INNER,
	};
	rpmsg_id_t rpmsg_id = rpmsg_init(RPMSG_HOST, shm_mem, buf_info);
	int data = 0, ret = 0;

	rpmsg_msg_cb_register(rpmsg_id, rpmsg_msg_callback);
	arch_irq_connect(USER_IPI_MIN, 160, (irq_routine_t)rpmsg_receive_handler,
					 (const void *)rpmsg_id, IRQ_TYPE_LEVEL);
	srand(0);

	for (;;) {
		data = rand();
		ret = rpmsg_send(rpmsg_id, &data, sizeof(data));
		if (ret < 0) {
			printf("send data ret = %d\n", ret);
		}
		task_delay(1);
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

static int rpmsg_notify_cb(int32_t ipi, int32_t cpu_id) {
	gic_raise_sgi(ipi, 0, BIT(cpu_id));

	return 0;
}

static void test_task2_entry(void *arg0, void *arg1, void *arg2, void *arg3) {
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

	struct shm_mem_info shm_mem = {
		.virt = (void *)buffer,
		.phys = virt_to_phys(buffer),
		.size = BUFFER_SIZE,
		.sync_type = SHM_SYNC_INNER,
	};
	rpmsg_id_t rpmsg_id = rpmsg_init(RPMSG_REMOTE, shm_mem, buf_info);
	int32_t data = 0, ret = 0;

	rpmsg_notify_register(rpmsg_id, (rpmsg_notify)rpmsg_notify_cb,
						  (void *)USER_IPI_MIN, (void *)1);

	for (;;) {
		data = 0;
		ret = rpmsg_receive(rpmsg_id, &data, sizeof(data));
		if (ret > 0) {
			rpmsg_send(rpmsg_id, &data, sizeof(data));
		}
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

int main() {
	printf("enter root task\n");
	memset(buffer, 0, BUFFER_SIZE);
	create_test_task1();
	create_test_task2();

	return 0;
}
