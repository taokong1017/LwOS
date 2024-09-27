
#include <types.h>
#include <task.h>
#include <task_sched.h>
#include <msgq.h>
#include <mem_mgr.h>
#include <string.h>
#include <menuconfig.h>
#include <list.h>
#include <log.h>
#include <tick.h>

#define MSGQ_TAG "MSGQ"
#define MSGQ_INIT_TIME -1
#define MSGQ_INVALID_ID -1
#define min(a, b) ((a) < (b) ? (a) : (b))

#define msgq2id(msgq) ((msgq_id_t)msgq)
#define id2msgq(id) ((struct msgq *)id)

/*
 * when put threee messages to buffer in the msgq, it will been layout as
 * follows:
 * --------------------------------------------------------------------------
 * |-- the length of msg1 --|---------------- msg1 content -----------------|
 * |-- the length of msg2 --|---------------- msg2 content -----------------|
 * |-- the length of msg3 --|---------------- msg3 content -----------------|
 * --------------------------------------------------------------------------
 */
#define msgq_buffer_size(msg_size) ((msg_size) + sizeof(uint32_t))
#define msgq_buffer_len_addr(buffer) ((uint32_t *)buffer)
#define msgq_buffer_msg_addr(buffer) ((void *)buffer + sizeof(uint32_t))

errno_t msgq_create(const char *name, uint32_t max_msgs, uint32_t max_msg_size,
					msgq_id_t *id) {
	struct msgq *msgq = NULL;
	char *buffer = NULL;

	if (!name) {
		log_err(MSGQ_TAG, "the msgq name is empty\n");
		return ERRNO_MSGQ_NAME_EMPTY;
	}

	if (max_msgs == 0) {
		log_err(MSGQ_TAG, "the maximum message number is zero\n");
		return ERRNO_MSGQ_ZERO_NUM;
	}
	if (max_msgs > CONFIG_MSGQ_MAX_NUM) {
		log_err(MSGQ_TAG, "the maximum message number is larger than %d\n",
				CONFIG_MSGQ_MAX_NUM);
		return ERRNO_MSGQ_OVERFLOW_MAX_NUM;
	}

	if (max_msg_size == 0) {
		log_err(MSGQ_TAG, "the maximum message size is zero\n");
		return ERRNO_MSGQ_ZERO_SIZE;
	}
	if (max_msg_size > CONFIG_MSGQ_MAX_SIZE) {
		log_err(MSGQ_TAG, "the maximum message size is larger than %d\n",
				CONFIG_MSGQ_MAX_SIZE);
		return ERRNO_MSGQ_OVERFLOW_MAX_SIZE;
	}

	if (!id) {
		log_err(MSGQ_TAG, "the input id is NULL\n");
		return ERRNO_MSGQ_PTR_NULL;
	}

	msgq = (struct msgq *)mem_malloc(sizeof(struct msgq));
	if (!msgq) {
		log_err(MSGQ_TAG, "malloc memory failed for creating msgq\n");
		return ERRNO_MSGQ_NO_MEMORY;
	}

	buffer = (char *)mem_malloc(max_msgs * msgq_buffer_size(max_msg_size));
	if (!msgq) {
		mem_free(msgq);
		log_err(MSGQ_TAG, "malloc memory failed for creating buffer\n");
		return ERRNO_MSGQ_NO_MEMORY;
	}

	strncpy((void *)msgq->name, (void *)name, MSGQ_NAME_LEN);
	msgq->name[MSGQ_NAME_LEN - 1] = '\0';
	msgq->msg_stime = MSGQ_INIT_TIME;
	msgq->msg_rtime = MSGQ_INIT_TIME;
	msgq->msg_ctime = MSGQ_INIT_TIME;
	msgq->max_msgs = max_msgs;
	msgq->max_msg_size = max_msg_size;
	msgq->msg_buffer = buffer;
	msgq->msg_head = 0;
	msgq->msg_tail = 0;
	msgq->msg_used = 0;
	msgq->id = msgq2id(msgq);
	*id = msgq->id;

	INIT_LIST_HEAD(&msgq->rec_queue.wait_list);
	INIT_LIST_HEAD(&msgq->send_queue.wait_list);
	log_debug(MSGQ_TAG, "the msgq %s is created, id is 0x%lx\n", name,
			  msgq->id);

	return OK;
}

errno_t msgq_destroy(msgq_id_t id) {
	struct msgq *msgq = id2msgq(id);

	if (!msgq || msgq->id != id) {
		log_err(MSGQ_TAG, "the msgq id 0x%lx, is invalid\n", id);
		return ERRNO_MSGQ_ID_IVALID;
	}

	if (msgq->msg_used > 0) {
		log_info(MSGQ_TAG, "the msgq is been used\n");
		return ERRNO_MSGQ_IS_BUSY;
	}

	msgq->id = MSGQ_INVALID_ID;
	list_del_init(&msgq->rec_queue.wait_list);
	list_del_init(&msgq->send_queue.wait_list);

	mem_free(msgq->msg_buffer);
	mem_free(msgq);
	log_debug(MSGQ_TAG, "the msgq is destroyed, id is 0x%lx\n", id);

	return OK;
}

static void msgq_send_copy(struct msgq *msgq, const void *msg, uint32_t size) {
	void *msgq_buffer = NULL;

	msgq_buffer = msgq->msg_buffer +
				  msgq->msg_head * msgq_buffer_size(msgq->max_msg_size);
	size = min(size, msgq->max_msg_size);
	*msgq_buffer_len_addr(msgq_buffer) = size;
	memcpy(msgq_buffer_msg_addr(msgq_buffer), msg, size);

	msgq->msg_used++;
	msgq->msg_head = (msgq->msg_head + 1) % msgq->max_msgs;

	return;
}

errno_t msgq_send(msgq_id_t id, const void *msg, uint32_t size,
				  uint64_t timeout) {
	uint64_t current_ticks = current_ticks_get();
	struct msgq *msgq = id2msgq(id);
	uint32_t key = 0;
	errno_t ret = OK;

	if (!msgq || msgq->id != id) {
		log_err(MSGQ_TAG, "the msgq id is invalid\n");
		return ERRNO_MSGQ_ID_IVALID;
	}

	if (!msg) {
		log_err(MSGQ_TAG, "the msg is NULL\n");
		return ERRNO_MSGQ_PTR_NULL;
	}

	if (size == 0) {
		log_err(MSGQ_TAG, "the msg size is zero\n");
		return ERRNO_MSGQ_ZERO_SIZE;
	}

	if (size > msgq->max_msg_size) {
		log_info(MSGQ_TAG, "the msg size is larger than maximum size %d\n",
				 msgq->max_msg_size);
	}

	key = sched_spin_lock();

	if (msgq->msg_used < msgq->max_msgs) {
		msgq->msg_stime = current_ticks;
		msgq_send_copy(msgq, msg, size);
		task_wakeup_locked(&msgq->rec_queue);
	} else if (timeout == MSGQ_NO_WAIT) {
		ret = ERRNO_MSGQ_QUEUE_FULL;
	} else {
		msgq->msg_stime = current_ticks;
		ret = task_wait_locked(&msgq->send_queue, timeout, true);
		msgq->msg_ctime = MSGQ_INIT_TIME;
		if (ret == OK) {
			msgq_send_copy(msgq, msg, size);
			msgq->msg_ctime = current_ticks_get();
		} else { /* only ERRNO_TASK_WAIT_TIMEOUT */
			ret = ERRNO_MSGQ_TIMEOUT;
		}
	}

	sched_spin_unlock(key);

	return ret;
}

static void msgq_receive_copy(struct msgq *msgq, void *msg, uint32_t *size) {
	void *msgq_buffer = NULL;

	msgq_buffer = msgq->msg_buffer +
				  msgq->msg_tail * msgq_buffer_size(msgq->max_msg_size);
	*size = min(*size, *msgq_buffer_len_addr(msgq_buffer));
	memcpy(msg, msgq_buffer_msg_addr(msgq_buffer), *size);

	msgq->msg_used--;
	msgq->msg_tail = (msgq->msg_tail + 1) % msgq->max_msgs;

	return;
}

errno_t msgq_receive(msgq_id_t id, void *msg, uint32_t *size,
					 uint64_t timeout) {
	uint64_t current_ticks = current_ticks_get();
	struct msgq *msgq = id2msgq(id);
	uint32_t key = 0;
	errno_t ret = OK;

	if (!msgq || msgq->id != id) {
		log_err(MSGQ_TAG, "the msgq id is invalid\n");
		return ERRNO_MSGQ_ID_IVALID;
	}

	if (!msg || !size) {
		log_err(MSGQ_TAG, "the msg or size is NULL\n");
		return ERRNO_MSGQ_PTR_NULL;
	}

	if (*size == 0) {
		log_err(MSGQ_TAG, "the msg size is zero\n");
		return ERRNO_MSGQ_ZERO_SIZE;
	}

	log_debug(MSGQ_TAG, "the msg size is %d\n", msgq->max_msg_size);

	key = sched_spin_lock();

	if (msgq->msg_used > 0) {
		msgq->msg_rtime = current_ticks;
		msgq_receive_copy(msgq, msg, size);
		task_wakeup_locked(&msgq->send_queue);
	} else if (timeout == MSGQ_NO_WAIT) {
		ret = ERRNO_MSGQ_QUEUE_EMPTY;
	} else {
		msgq->msg_rtime = current_ticks;
		ret = task_wait_locked(&msgq->rec_queue, timeout, true);
		msgq->msg_ctime = MSGQ_INIT_TIME;
		if (ret == OK) {
			msgq_receive_copy(msgq, msg, size);
			msgq->msg_ctime = current_ticks_get();
		} else { /* only ERRNO_TASK_WAIT_TIMEOUT */
			ret = ERRNO_MSGQ_TIMEOUT;
		}
	}

	sched_spin_unlock(key);

	return ret;
}
