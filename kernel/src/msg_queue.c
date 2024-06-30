
#include <types.h>
#include <task.h>
#include <task_sched.h>
#include <msg_queue.h>
#include <memory.h>
#include <string.h>
#include <menuconfig.h>
#include <list.h>

#define MSGQ_TAG "MSG_QUEUE"
#define MSGQ_MAX_NUM 1024
#define MSGQ_MAX_MSG_SIZE 1024
#define MSGQ_INIT_TIME -1
#define MSGQ_INVALID_ID -1

#define msgq2id(msg) ((msgq_id_t)msg)
#define id2msgq(id) ((struct msg_queue *)id)

errno_t msg_queue_create(const char *name, uint32_t max_msgs,
						 uint32_t max_msg_size, msgq_id_t *id) {
	struct msg_queue *msgq = NULL;
	char *buffer = NULL;

	if (!name) {
		return ERRNO_MSGQ_NAME_EMPTY;
	}

	if (max_msgs == 0) {
		return ERRNO_MSGQ_ZERO_NUM;
	}
	if (max_msgs > CONFIG_MSGQ_MAX_NUM) {
		return ERRNO_MSGQ_OVERFLOW_MAX_NUM;
	}

	if (max_msg_size == 0) {
		return ERRNO_MSGQ_ZERO_SIZE;
	}
	if (max_msg_size > CONFIG_MSGQ_MAX_SIZE) {
		return ERRNO_MSGQ_OVERFLOW_MAX_SIZE;
	}

	if (!id) {
		return ERRNO_MSGQ_PTR_NULL;
	}

	msgq = (struct msg_queue *)mem_alloc(sizeof(struct msg_queue));
	if (!msgq) {
		return ERRNO_MSGQ_NO_MEMORY;
	}

	buffer = (char *)mem_alloc(max_msgs * max_msg_size);
	if (!msgq) {
		mem_free(msgq);
		return ERRNO_MSGQ_NO_MEMORY;
	}

	strncpy((void *)msgq->name, (void *)name, MSGQ_NAME_LEN);
	msgq->msg_stime = MSGQ_INIT_TIME;
	msgq->msg_rtime = MSGQ_INIT_TIME;
	msgq->msg_ctime = MSGQ_INIT_TIME;
	msgq->max_msgs = max_msgs;
	msgq->max_msg_size = max_msg_size;
	msgq->msg_buffer = buffer;
	msgq->msg_head = 0;
	msgq->msg_tail = 0;
	msgq->id = msgq2id(msgq);
	*id = msgq->id;

	INIT_LIST_HEAD(&msgq->rec_queue.wait_list);
	INIT_LIST_HEAD(&msgq->send_queue.wait_list);

	return OK;
}

errno_t msg_queue_destroy(msgq_id_t id) {
	struct msg_queue *msgq = id2msgq(id);

	if (!msgq) {
		return ERRNO_MSGQ_ID_IVALID;
	}

	// TODO: wait for message released

	list_del_init(&msgq->rec_queue.wait_list);
	list_del_init(&msgq->send_queue.wait_list);

	mem_free(msgq->msg_buffer);
	mem_free(msgq);

	return OK;
}

errno_t msg_queue_send(msgq_id_t id, const void *msg, uint32_t size,
					   uint32_t timeout) {
	return OK;
}

errno_t msg_queue_receive(msgq_id_t id, const void *msg, uint32_t *size,
						  uint32_t timeout) {
	return OK;
}
