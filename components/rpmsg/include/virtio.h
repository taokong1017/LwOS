#ifndef __VIRT_IO_H__
#define __VIRT_IO_H__

#include <virtqueue.h>
#include <metal_errno.h>

/* Status byte for guest to report progress. */
#define VIRTIO_CONFIG_STATUS_RESET 0x00
#define VIRTIO_CONFIG_STATUS_DRIVER_OK 0x01
#define VIRTIO_CONFIG_STATUS_FAILED 0x02

/* Virtio device role */
#define VIRTIO_DEV_DRIVER 0UL
#define VIRTIO_DEV_DEVICE 1UL

/* @brief Virtio vring data structure */
struct virtio_vring_info {
	/* Virtio queue */
	struct virtqueue *vq;

	/* Vring alloc info */
	struct vring_alloc_info info;

	/* Metal I/O region of the vring memory, can be NULL */
	struct metal_io_region *io;
};

/**
 * @brief Represents a Virtio device, which can be either a backend or a
 * frontend.
 *
 * This structure contains information about a Virtio device, including its role
 * (backend or frontend), a dispatch table of Virtio functions, private data,
 * the number of Virtio rings, and pointers to the Virtio ring information.
 */
struct virtio_device {
	/* If it is virtio backend or front end. */
	unsigned int role;

	/* Virtio dispatch table */
	const struct virtio_dispatch *func;

	/* Private data */
	void *priv;

	/* Number of vrings */
	unsigned int vrings_num;

	/* Pointer to the virtio vring structure */
	struct virtio_vring_info *vrings_info;
};

/**
 * @brief Virtio device dispatcher functions.
 *
 * The virtio transport layers are expected to implement these functions in
 * their respective codes.
 */
struct virtio_dispatch {
	/* Get the status of the virtio device. */
	uint8_t (*get_status)(struct virtio_device *dev);

	/* Set the status of the virtio device. */
	void (*set_status)(struct virtio_device *dev, uint8_t status);

	/* Notify the other side that a virtio vring as been updated. */
	void (*notify)(struct virtqueue *vq);
};

/**
 * @brief Create the virtio device virtqueue.
 *
 * @param vdev			Pointer to virtio device structure.
 * @param flags			Create flag.
 * @param nvqs			The virtqueue number.
 * @param names			Virtqueue names.
 * @param callbacks		Virtqueue callback functions.
 * @param callback_args	Virtqueue callback function arguments.
 *
 * @return 0 on success, otherwise error code.
 */
int virtio_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
							 unsigned int nvqs, const char *names[],
							 vq_callback callbacks[], void *callback_args[]);

#endif
