
#include <types.h>
#include <task.h>
#include <task_sched.h>
#include <msg_queue.h>

errno_t msg_queue_create(const char *name, uint32_t max_msgs, uint32_t msg_size,
						 msgq_id_t *id) {
	return OK;
}

errno_t msg_queue_destroy(msgq_id_t id) { return OK; }

errno_t msg_queue_send(msgq_id_t id, const void *msg, uint32_t size,
					   uint32_t timeout) {
	return OK;
}

errno_t msg_queue_receive(msgq_id_t id, const void *msg, uint32_t *size,
						  uint32_t timeout) {
	return OK;
}
