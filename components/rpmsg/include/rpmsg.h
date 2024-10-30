#ifndef __RPMSG_H__
#define __RPMSG_H__

#include <metal_mutex.h>
#include <metal_atomic.h>
#include <virtio.h>

#define INVALID_RPMSG_ID -1
#define RPMSG_LOCATE_HDR(p)                                                    \
	((struct rpmsg_hdr *)((unsigned char *)(p) - sizeof(struct rpmsg_hdr)))
#define RPMSG_LOCATE_DATA(p) ((unsigned char *)(p) + sizeof(struct rpmsg_hdr))

#define metal_offset_of(structure, member)                                     \
	((uintptr_t) & (((structure *)0)->member))
#define metal_container_of(ptr, structure, member)                             \
	(void *)((uintptr_t)(ptr)-metal_offset_of(structure, member))
#define RPMSG_ASSERT(_exp, _msg)

#define RPMSG_SUCCESS 0
#define RPMSG_ERROR_BASE -2000
#define RPMSG_ERR_NO_MEM (RPMSG_ERROR_BASE - 1)
#define RPMSG_ERR_NO_BUFF (RPMSG_ERROR_BASE - 2)
#define RPMSG_ERR_PARAM (RPMSG_ERROR_BASE - 3)
#define RPMSG_ERR_DEV_STATE (RPMSG_ERROR_BASE - 4)
#define RPMSG_ERR_BUFF_SIZE (RPMSG_ERROR_BASE - 5)
#define RPMSG_ERR_INIT (RPMSG_ERROR_BASE - 6)
#define RPMSG_ERR_ADDR (RPMSG_ERROR_BASE - 7)
#define RPMSG_ERR_PERM (RPMSG_ERROR_BASE - 8)

#define RPMSG_REMOTE VIRTIO_DEV_DEVICE
#define RPMSG_HOST VIRTIO_DEV_DRIVER

typedef int (*rpmsg_msg_cb)(void *data, uint32_t len);
typedef int (*rpmsg_notify)(void *arg0, void *arg1);
typedef unsigned long phys_addr_t;
typedef unsigned long rpmsg_id_t;
struct rpmsg_device;

/**
 * @brief Holds information about an RPMSG notification callback.
 *
 * This structure contains the necessary information to register an RPMSG
 * notification callback, including the callback function pointer and any
 * associated arguments.
 *
 * @param notify The callback function to be invoked when the notification
 *               is received.
 * @param arg0   The first argument to be passed to the callback function.
 * @param arg1   The second argument to be passed to the callback function.
 */
struct rpmsg_notify_info {
	rpmsg_notify notify;
	void *arg0;
	void *arg1;
};

/**
 * @brief Common header for all RPMsg messages
 *
 * Every message sent(/received) on the RPMsg bus begins with this header.
 * The header contains the length of the message in bytes and a reserved field
 * for future use.
 */
struct rpmsg_hdr {
	uint16_t len;
	uint16_t reserved;
};

/**
 * @brief RPMsg operations
 *
 * This struct defines the set of operations that can be performed on an RPMsg
 * device.
 *
 * @param send Sends data through the RPMsg device
 * @param receive Receives data from the RPMsg device
 */
struct rpmsg_ops {
	int (*send)(struct rpmsg_device *rdev, void *data, uint32_t len);
	int (*receive)(struct rpmsg_device *rdev, void *buffer, uint32_t len);
};

/**
 * @brief Representation of an RPMsg device
 *
 * This struct represents an RPMsg device, which is used for communication over
 * the RPMsg bus. It contains a lock, a set of RPMsg operations, and a callback
 * function for handling incoming RPMsg messages.
 *
 * @param lock			A metal mutex used to synchronize access to the RPMsg
 * device
 * @param ops			A set of RPMsg operations that can be performed on the
 * device
 * @param rpmsg_msg_cb	A callback function that is called when an RPMsg message
 * is received
 */
struct rpmsg_device {
	metal_mutex_t lock;
	struct rpmsg_ops *ops;
	rpmsg_msg_cb rpmsg_msg_cb;
};

/**
 * @brief Represents the shared memory information used by RPMsg.
 *
 * This structure contains the virtual address, physical address, and size of
 * the shared memory used by the RPMsg subsystem.
 *
 * @param virt	The base virtual address of the shared memory.
 * @param phys	The base physical address of the shared memory.
 * @param size	The size of the shared memory in bytes.
 */
struct shm_mem_info {
	void *virt;
	phys_addr_t phys;
	uint32_t size;
	enum SHM_SYNC_TYPE sync_type;
};

/**
 * @brief Represents the buffer information used by RPMsg.
 *
 * This structure contains the number and size of the buffers used for
 * communication from host to remote and from remote to host.
 *
 * @param h2r_buf_num The number of buffers used for communication from host to
 * remote.
 * @param h2r_buf_size The size of each buffer used for communication from host
 * to remote.
 * @param r2h_buf_num The number of buffers used for communication from remote
 * to host.
 * @param r2h_buf_size The size of each buffer used for communication from
 * remote to host.
 */
struct rpmsg_buf_info {
	uint32_t h2r_buf_num;
	uint32_t h2r_buf_size;
	uint32_t r2h_buf_num;
	uint32_t r2h_buf_size;
};

/**
 * @brief Send an RPMsg message
 *
 * Sends an RPMsg message to the other processor through the system shared
 * memory.
 *
 * @param id	The RPMsg identifier.
 * @param data	Pointer to the message data to be sent.
 * @param len	The length of the message data in bytes.
 *
 * @return The number of bytes sent, or a negative error code on failure.
 */
int32_t rpmsg_send(rpmsg_id_t id, void *data, uint32_t len);

/**
 * @brief Receive an RPMsg message
 *
 * Receives an RPMsg message from the other processor through the system shared
 * memory.
 *
 * @param id		The RPMsg identifier.
 * @param buffer	Pointer to the buffer to store the received message data.
 * @param len 		The maximum length of the buffer in bytes.
 *
 * @return The number of bytes received, or a negative error code on failure.
 */
int32_t rpmsg_receive(rpmsg_id_t id, void *buffer, uint32_t len);

/**
 * @brief To register the callback to receive RPMsg messages
 *
 * register the passively the RPMsg receiving messages from the other through
 * system shared memory.
 * @param id	The RPMsg identifier.
 * @param cb	The RPMsg callback function to receive RPMsg messages.
 *
 * @return the status of the register operation.
 */
int32_t rpmsg_msg_cb_register(rpmsg_id_t id, rpmsg_msg_cb cb);

/**
 * @brief Handle the reception of an RPMsg message
 *
 * This function is called when an RPMsg message has been received for the
 * specified RPMsg identifier. It is typically used as a callback function
 * registered with the rpmsg_msg_cb_register() function.
 *
 * @param id The RPMsg identifier for which the message was received.
 */
void rpmsg_receive_handler(rpmsg_id_t id);

/**
 * @brief Register a notification callback for an RPMsg identifier
 *
 * Registers a callback function that will be called when an RPMsg message is
 * sent for the specified identifier. The callback function be used to notify
 * the remote to handle the sent message.
 *
 * @param id    The RPMsg identifier.
 * @param notify The callback function to be called to notify the remote.
 * @param arg0  An first argument to be passed to the callback function.
 * @param arg1  An second argument to be passed to the callback fucntion.
 *
 * @return The status of the registration operation.
 */
int32_t rpmsg_notify_register(rpmsg_id_t id, rpmsg_notify notify, void *arg0,
							  void *arg1);

/**
 * @brief Initialize the RPMsg subsystem
 *
 * This function initializes the RPMsg subsystem with the specified role, shared
 * memory information, and buffer information.
 *
 * @param role		The RPMsg role (e.g. RPMSG_REMOTE or RPMSG_HOST)
 * @param shm		The shared memory information for the RPMsg subsystem
 * @param buf_info	The buffer information for the RPMsg subsystem
 *
 * @return The RPMsg identifier for the initialized subsystem
 */
rpmsg_id_t rpmsg_init(int32_t role, struct shm_mem_info shm,
					  struct rpmsg_buf_info buf_info);

/**
 * @brief Deinitialize the RPMsg subsystem
 *
 * This function deinitializes the RPMsg subsystem with the specified
 * identifier.
 *
 * @param id The RPMsg identifier to deinitialize
 */
void rpmsg_deinit(rpmsg_id_t id);

#endif
