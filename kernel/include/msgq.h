#ifndef __MSGQ_H__
#define __MSGQ_H__

#include <types.h>
#include <errno.h>
#include <kernel.h>

#define MSGQ_INVALID_ID -1
#define MSGQ_NAME_LEN 32
#define MSGQ_WAIT_FOREVER WAIT_FOREVER
#define MSGQ_NO_WAIT NO_WAIT

/* msgq error code definition */
#define ERRNO_MSGQ_NAME_EMPTY ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x00)
#define ERRNO_MSGQ_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_MSGQ, 0x01)
#define ERRNO_MSGQ_PTR_NULL ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x02)
#define ERRNO_MSGQ_ZERO_SIZE ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x03)
#define ERRNO_MSGQ_OVERFLOW_MAX_SIZE ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x04)
#define ERRNO_MSGQ_ZERO_NUM ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x05)
#define ERRNO_MSGQ_OVERFLOW_MAX_NUM ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x06)
#define ERRNO_MSGQ_ID_IVALID ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x07)
#define ERRNO_MSGQ_IS_BUSY ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x08)
#define ERRNO_MSGQ_QUEUE_FULL ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x09)
#define ERRNO_MSGQ_QUEUE_EMPTY ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x09)
#define ERRNO_MSGQ_TIMEOUT ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x0a)
#define ERRNO_MSGQ_NO_ENOUGH_SIZE ERRNO_OS_ERROR(MOD_ID_MSGQ, 0x0b)

typedef long msgq_id_t;

struct msgq {
	msgq_id_t id;
	char name[MSGQ_NAME_LEN];

	uint64_t msg_stime; /* msg send time */
	uint64_t msg_rtime; /* msgq receive time */
	uint64_t msg_ctime; /* msg change time */

	uint32_t max_msg_size; /* maximum msg size */
	uint32_t max_msgs;	   /* maximum msg number */

	char *msg_buffer;  /* msg buffer pointer */
	uint32_t msg_head; /* msg head */
	uint32_t msg_tail; /* msg tail */
	uint32_t msg_used; /* msg used count */

	struct wait_queue rec_queue;  /* msg receive wait queue */
	struct wait_queue send_queue; /* msg send wait queue */
};

errno_t msgq_create(const char *name, uint32_t max_msgs, uint32_t msg_size,
					msgq_id_t *id);
errno_t msgq_send(msgq_id_t id, const void *msg, uint32_t size,
				  uint64_t timeout);
errno_t msgq_receive(msgq_id_t id, void *msg, uint32_t *size, uint64_t timeout);
errno_t msgq_destroy(msgq_id_t id);

#endif
