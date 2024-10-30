#ifndef __VIRTQUEUE_H__
#define __VIRTQUEUE_H__

#include <virtio_ring.h>
#include <metal_alloc.h>
#include <metal_io.h>
#include <string.h>
#include <metal_cache.h>

/* Error Codes */
#define VQ_ERROR_BASE -3000
#define ERROR_VRING_FULL (VQ_ERROR_BASE - 1)
#define ERROR_INVLD_DESC_IDX (VQ_ERROR_BASE - 2)
#define ERROR_EMPTY_RING (VQ_ERROR_BASE - 3)
#define ERROR_NO_MEM (VQ_ERROR_BASE - 4)
#define ERROR_VRING_MAX_DESC (VQ_ERROR_BASE - 5)
#define ERROR_VRING_ALIGN (VQ_ERROR_BASE - 6)
#define ERROR_VRING_NO_BUFF (VQ_ERROR_BASE - 7)
#define ERROR_VQUEUE_INVLD_PARAM (VQ_ERROR_BASE - 8)

#define VQUEUE_SUCCESS 0

/* The maximum virtqueue size is 2^15. Use that value as the end of
 * descriptor chain terminator since it will never be a valid index
 * in the descriptor table. This is used to verify we are correctly
 * handling vq_free_cnt.
 */
#define VQ_RING_DESC_CHAIN_END 32768

/* cache invalidation helpers */
#define VRING_FLUSH(x, s) metal_cache_flush(x, s)
#define VRING_INVALIDATE(x, s) metal_cache_invalidate(x, s)

/* @brief Buffer descriptor. */
struct virtqueue_buf {
	/* Address of the buffer. */
	void *buf;

	/* Size of the buffer. */
	int len;
};

/* @brief Vring descriptor extra information for buffer list management. */
struct vq_desc_extra {
	/* Pointer to first descriptor. */
	void *cookie;

	/* Number of chained descriptors. */
	uint16_t ndescs;
};

/* @brief Local virtio queue to manage a virtio ring for sending or receiving.
 */
struct virtqueue {
	/* Associated virtio device. */
	struct virtio_device *vq_dev;

	/* Name of the virtio queue. */
	const char *vq_name;

	/* Index of the virtio queue. */
	uint16_t vq_queue_index;

	/* Max number of buffers in the virtio queue. */
	uint16_t vq_nentries;

	/* Function to invoke, when message is available on the virtio queue. */
	void (*callback)(struct virtqueue *vq);

	/* Function to invoke, to inform the other side about an update in the
	 * virtio queue. */
	void (*notify)(struct virtqueue *vq);

	/* Associated virtio ring. */
	struct vring vq_ring;

	/* Number of free descriptor in the virtio ring. */
	uint16_t vq_free_cnt;

	/**
	 * Metal I/O region of the vrings and buffers.
	 * This structure is used for conversion between virtual and physical
	 * addresses.
	 */
	void *shm_io;

	/**
	 * Head of the free chain in the descriptor table. If there are no free
	 * descriptors, this will be set to VQ_RING_DESC_CHAIN_END.
	 */
	uint16_t vq_desc_head_idx;

	/* Last consumed descriptor in the used table, trails vq_ring.used->idx. */
	uint16_t vq_used_cons_idx;

	/* Last consumed descriptor in the available table, used by the consumer
	 * side. */
	uint16_t vq_available_idx;

	/**
	 * Used by the host side during callback. Cookie holds the address of buffer
	 * received from other side. Other fields in this structure are not used
	 * currently.
	 */
	struct vq_desc_extra vq_descx[0];
};

/* @brief Virtio ring specific information. */
struct vring_alloc_info {
	/* Vring address. */
	void *vaddr;

	/* Vring alignment. */
	uint32_t align;

	/* Number of descriptors in the vring. */
	uint16_t num_descs;
};

typedef void (*vq_callback)(struct virtqueue *);
typedef void (*vq_notify)(struct virtqueue *);

/**
 * @internal
 *
 * @brief Creates new VirtIO queue
 *
 * @param device	Pointer to VirtIO device
 * @param id		VirtIO queue ID , must be unique
 * @param name		Name of VirtIO queue
 * @param ring		Pointer to vring_alloc_info control block
 * @param callback	Pointer to callback function, invoked
 *			when message is available on VirtIO queue
 * @param notify	Pointer to notify function, used to notify
 *			other side that there is job available for it
 * @param vq		Created VirtIO queue.
 *
 * @return Function status
 */
int virtqueue_create(struct virtio_device *device, unsigned short id,
					 const char *name, struct vring_alloc_info *ring,
					 void (*callback)(struct virtqueue *vq),
					 void (*notify)(struct virtqueue *vq),
					 struct virtqueue *vq);

/**
 * @internal
 *
 * @brief Enqueues new buffer in vring for consumption by other side. Readable
 * buffers are always inserted before writable buffers
 *
 * @param vq		Pointer to VirtIO queue control block.
 * @param buf_list	Pointer to a list of virtqueue buffers.
 * @param readable	Number of readable buffers
 * @param writable	Number of writable buffers
 * @param cookie	Pointer to hold call back data
 *
 * @return Function status
 */
int virtqueue_add_buffer(struct virtqueue *vq, struct virtqueue_buf *buf_list,
						 int readable, int writable, void *cookie);

/**
 * @internal
 *
 * @brief Returns used buffers from VirtIO queue
 *
 * @param vq	Pointer to VirtIO queue control block
 * @param len	Length of conumed buffer
 * @param idx	Index of the buffer
 *
 * @return Pointer to used buffer
 */
void *virtqueue_get_buffer(struct virtqueue *vq, uint32_t *len, uint16_t *idx);

/**
 * @internal
 *
 * @brief Returns buffer available for use in the VirtIO queue
 *
 * @param vq		Pointer to VirtIO queue control block
 * @param avail_idx	Pointer to index used in vring desc table
 * @param len		Length of buffer
 *
 * @return Pointer to available buffer
 */
void *virtqueue_get_available_buffer(struct virtqueue *vq, uint16_t *avail_idx,
									 uint32_t *len);

/**
 * @internal
 *
 * @brief Returns consumed buffer back to VirtIO queue
 *
 * @param vq        Pointer to VirtIO queue control block
 * @param head_idx  Index of vring desc containing used buffer
 * @param len       Length of buffer
 *
 * @return Function status
 */
int virtqueue_add_consumed_buffer(struct virtqueue *vq, uint16_t head_idx,
								  uint32_t len);

/**
 * @internal
 *
 * @brief Disables callback generation
 *
 * @param vq	Pointer to VirtIO queue control block
 */
void virtqueue_disable_cb(struct virtqueue *vq);

/**
 * @internal
 *
 * @brief Enables callback generation
 *
 * @param vq	Pointer to VirtIO queue control block
 *
 * @return Function status
 */
int virtqueue_enable_cb(struct virtqueue *vq);

/**
 * @internal
 *
 * @brief Notifies other side that there is buffer available for it.
 *
 * @param vq	Pointer to VirtIO queue control block
 */
void virtqueue_kick(struct virtqueue *vq);

static inline struct virtqueue *
virtqueue_allocate(unsigned int num_desc_extra) {
	struct virtqueue *vqs;
	uint32_t vq_size = sizeof(struct virtqueue) +
					   num_desc_extra * sizeof(struct vq_desc_extra);

	vqs = (struct virtqueue *)metal_allocate_memory(vq_size);
	if (vqs) {
		memset(vqs, 0x00, vq_size);
	}

	return vqs;
}

/**
 * @internal
 *
 * @brief Notifies other side that there is buffer available for it.
 *
 * @param vq	Pointer to VirtIO queue control block
 * @param idx	Index of vring desc containing used buffer
 *
 * @return Buffer length
 */
uint32_t virtqueue_get_buffer_length(struct virtqueue *vq, uint16_t idx);

/**
 * @internal
 *
 * @brief Notifies other side that there is buffer available for it.
 *
 * @param vq	Pointer to VirtIO queue control block
 * @param idx	Index of vring desc containing used buffer
 *
 * @return Buffer
 */
void *virtqueue_get_buffer_addr(struct virtqueue *vq, uint16_t idx);

/**
 * @brief Test if virtqueue is empty
 *
 * @param vq	Pointer to VirtIO queue control block
 *
 * @return 1 if virtqueue is empty, 0 otherwise
 */
static inline int virtqueue_empty(struct virtqueue *vq) {
	return (vq->vq_nentries == vq->vq_free_cnt);
}

/**
 * @brief Test if virtqueue is full
 *
 * @param vq	Pointer to VirtIO queue control block
 *
 * @return 1 if virtqueue is full, 0 otherwise
 */
static inline int virtqueue_full(struct virtqueue *vq) {
	return (vq->vq_free_cnt == 0);
}

/**
 * @brief Notifies other side that there is buffer available for it.
 *
 * @param vq	Pointer to VirtIO queue control block
 */
void virtqueue_notification(struct virtqueue *vq);

#ifdef __cplusplus
}
#endif

#endif
