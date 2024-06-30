#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include <types.h>

#define MSGQ_INVALID_ID -1
#define MSGQ_NAME_LEN 32
#define MSGQ_WAIT_FOREVER WAIT_FOREVER

typedef long msgq_id_t;

struct msg_queue {
	char name[MSGQ_NAME_LEN];

	uint64_t msg_stime; /* msg send time */
	uint64_t msg_rtime; /* msgq receive time */
	uint64_t msg_ctime; /* msg change time */

	uint32_t msg_size; /* msg size */
	uint32_t max_msgs; /* maximum msg number */

	uint8_t *msg_buffer; /* msg buffer pointer */
	uint32_t msg_head;	 /* msg head */
	uint32_t msg_tail;	 /* msg tail */

	struct wait_queue rec_queue;  /* msg receive wait queue */
	struct wait_queue send_queue; /* msg send wait queue */
};

errno_t msg_queue_create(const char *name, uint32_t max_msgs, uint32_t msg_size,
						 msgq_id_t *id);
errno_t msg_queue_send(msgq_id_t id, const void *msg, uint32_t size,
					   uint32_t timeout);
errno_t msg_queue_receive(msgq_id_t id, const void *msg, uint32_t *size,
						  uint32_t timeout);
errno_t msg_queue_destroy(msgq_id_t id);

#endif
