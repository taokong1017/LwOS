#include <metal_alloc.h>
#include <rpmsg.h>
#include <rpmsg_virtio.h>
#include <virtqueue.h>
#include <mem_domain.h>

static int rpmsg_virtio_send(struct rpmsg_device *rdev, void *data,
							 uint32_t len);
static int rpmsg_virtio_receive(struct rpmsg_device *rdev, void *buffer,
								uint32_t len);
global_data_section static struct rpmsg_ops rpmsg_virtio_ops = {
	.send = rpmsg_virtio_send,
	.receive = rpmsg_virtio_receive,
};

void *rpmsg_virtio_shm_pool_get_buffer(struct rpmsg_virtio_shm_pool *shpool,
									   size_t size) {
	void *buffer = NULL;

	if (!shpool || size == 0 || shpool->avail < size)
		return NULL;
	buffer = (char *)shpool->base + shpool->size - shpool->avail;
	shpool->avail -= size;

	return buffer;
}

void rpmsg_virtio_init_shm_pool(struct rpmsg_virtio_shm_pool *shpool, void *shb,
								size_t size) {
	if (!shpool || !shb || size == 0)
		return;
	shpool->base = shb;
	shpool->size = size;
	shpool->avail = size;
}

/**
 * @internal
 *
 * @brief Places the used buffer back on the virtqueue.
 *
 * @param rvdev		Pointer to remote core
 * @param buffer	Buffer pointer
 * @param len		Buffer length
 * @param idx		Buffer index
 */
static void rpmsg_virtio_return_buffer(struct rpmsg_virtio_device *rvdev,
									   void *buffer, uint32_t len,
									   uint16_t idx) {
	unsigned int role = rpmsg_virtio_get_role(rvdev);

	BUFFER_INVALIDATE(buffer, len);

	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;

		(void)idx;
		/* Initialize buffer node */
		vqbuf.buf = buffer;
		vqbuf.len = len;
		virtqueue_add_buffer(rvdev->rvq, &vqbuf, 0, 1, buffer);
	}

	if (role == RPMSG_REMOTE) {
		(void)buffer;
		virtqueue_add_consumed_buffer(rvdev->rvq, idx, len);
	}
}

/**
 * @internal
 *
 * @brief Places buffer on the virtqueue for consumption by the other side.
 *
 * @param rvdev		Pointer to rpmsg virtio
 * @param buffer	Buffer pointer
 * @param len		Buffer length
 * @param idx		Buffer index
 *
 * @return Status of function execution
 */
static int rpmsg_virtio_enqueue_buffer(struct rpmsg_virtio_device *rvdev,
									   void *buffer, uint32_t len,
									   uint16_t idx) {
	unsigned int role = rpmsg_virtio_get_role(rvdev);

	BUFFER_FLUSH(buffer, len);

	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;
		(void)idx;

		/* Initialize buffer node */
		vqbuf.buf = buffer;
		vqbuf.len = len;
		return virtqueue_add_buffer(rvdev->svq, &vqbuf, 1, 0, buffer);
	}

	if (role == RPMSG_REMOTE) {
		(void)buffer;
		return virtqueue_add_consumed_buffer(rvdev->svq, idx, len);
	}

	return 0;
}

/**
 * @internal
 *
 * @brief Get buffer to transmit messages.
 *
 * @param rvdev	Pointer to rpmsg device
 * @param len	Length of returned buffer
 * @param idx	Buffer index
 *
 * @return Pointer to buffer.
 */
static void *rpmsg_virtio_get_tx_buffer(struct rpmsg_virtio_device *rvdev,
										uint32_t *len, uint16_t *idx) {
	unsigned int role = rpmsg_virtio_get_role(rvdev);
	void *data = NULL;

	if (role == RPMSG_HOST) {
		data = virtqueue_get_buffer(rvdev->svq, len, idx);
		if (!data && rvdev->svq->vq_free_cnt) {
			data = rpmsg_virtio_shm_pool_get_buffer(rvdev->shpool,
													rvdev->config.h2r_buf_size);
			*len = rvdev->config.h2r_buf_size;
			*idx = 0;
		}
	}

	if (role == RPMSG_REMOTE) {
		data = virtqueue_get_available_buffer(rvdev->svq, idx, len);
	}

	return data;
}

/**
 * @internal
 *
 * @brief Retrieves the received buffer from the virtqueue.
 *
 * @param rvdev	Pointer to rpmsg device
 * @param len	Size of received buffer
 * @param idx	Index of buffer
 *
 * @return Pointer to received buffer
 */
static void *rpmsg_virtio_get_rx_buffer(struct rpmsg_virtio_device *rvdev,
										uint32_t *len, uint16_t *idx) {
	unsigned int role = rpmsg_virtio_get_role(rvdev);
	void *data = NULL;

	if (role == RPMSG_HOST) {
		data = virtqueue_get_buffer(rvdev->rvq, len, idx);
	}

	if (role == RPMSG_REMOTE) {
		data = virtqueue_get_available_buffer(rvdev->rvq, idx, len);
	}

	/* Invalidate the buffer before returning it */
	if (data) {
		BUFFER_INVALIDATE(data, *len);
	}

	return data;
}

/**
 * @internal
 *
 * @brief Tx callback function.
 *
 * @param vq	Pointer to virtqueue on which Tx is has been
 *		completed.
 */
static void rpmsg_virtio_tx_callback(struct virtqueue *vq) { (void)vq; }

/**
 * @internal
 *
 * @brief Rx callback function.
 *
 * @param vq	Pointer to virtqueue on which messages is received
 */
static void rpmsg_virtio_rx_callback(struct virtqueue *vq) {
	struct virtio_device *vdev = vq->vq_dev;
	struct rpmsg_virtio_device *rvdev = vdev->priv;
	struct rpmsg_device *rdev = &rvdev->rdev;
	rpmsg_msg_cb rpmsg_msg_cb = rvdev->rdev.rpmsg_msg_cb;
	struct rpmsg_hdr *rp_hdr = NULL;
	uint32_t len = 0;
	uint16_t idx = 0;

	if (!rpmsg_msg_cb) {
		return;
	}

	/* Process the received data from remote node */
	metal_mutex_acquire(&rdev->lock);
	rp_hdr = rpmsg_virtio_get_rx_buffer(rvdev, &len, &idx);
	metal_mutex_release(&rdev->lock);

	while (rp_hdr) {
		/* Call registered callback function to send data to user */
		rpmsg_msg_cb(RPMSG_LOCATE_DATA(rp_hdr), rp_hdr->len);

		metal_mutex_acquire(&rdev->lock);
		rpmsg_virtio_return_buffer(rvdev, rp_hdr, len, idx);
		rp_hdr = rpmsg_virtio_get_rx_buffer(rvdev, &len, &idx);
		if (!rp_hdr) {
			virtqueue_kick(rvdev->rvq);
		}
		metal_mutex_release(&rdev->lock);
	}
}

static void *rpmsg_virtio_get_tx_msg_buffer(struct rpmsg_device *rdev,
											uint32_t *len) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct rpmsg_hdr *rp_hdr = NULL;
	uint16_t idx = 0;
	int status = RPMSG_SUCCESS;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	/* Validate device state */
	status = rpmsg_virtio_get_status(rvdev);
	if (!(status & VIRTIO_CONFIG_STATUS_DRIVER_OK)) {
		return NULL;
	}

	metal_mutex_acquire(&rdev->lock);
	rp_hdr = rpmsg_virtio_get_tx_buffer(rvdev, len, &idx);
	if (!rp_hdr) {
		metal_mutex_release(&rdev->lock);
		return NULL;
	}
	metal_mutex_release(&rdev->lock);

	/* Store the index into the reserved field to be used when sending */
	rp_hdr->reserved = idx;

	/* Actual data buffer size is vring buffer size minus header length */
	*len -= sizeof(struct rpmsg_hdr);

	return RPMSG_LOCATE_DATA(rp_hdr);
}

static int rpmsg_virtio_send_tx_msg_buffer(struct rpmsg_device *rdev,
										   const void *data, uint32_t len) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct metal_io_region *io = NULL;
	struct rpmsg_hdr rp_hdr;
	struct rpmsg_hdr *hdr = NULL;
	uint32_t buff_len = 0;
	uint16_t idx = 0;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	hdr = RPMSG_LOCATE_HDR(data);
	/* The reserved field contains buffer index */
	idx = hdr->reserved;

	/* Initialize RPMSG header. */
	rp_hdr.len = len;
	rp_hdr.reserved = 0;

	/* Copy data to rpmsg buffer. */
	io = rvdev->shbuf_io;
	metal_io_block_write(io, metal_io_virt_to_offset(io, hdr), &rp_hdr,
						 sizeof(rp_hdr));

	metal_mutex_acquire(&rdev->lock);

	if (rpmsg_virtio_get_role(rvdev) == RPMSG_HOST) {
		buff_len = rvdev->config.h2r_buf_size;
	} else {
		buff_len = virtqueue_get_buffer_length(rvdev->svq, idx);
	}

	/* Enqueue buffer on virtqueue. */
	rpmsg_virtio_enqueue_buffer(rvdev, hdr, buff_len, idx);
	/* Let the other side know that there is a job to process. */
	virtqueue_kick(rvdev->svq);

	metal_mutex_release(&rdev->lock);

	return len;
}

static int rpmsg_virtio_send(struct rpmsg_device *rdev, void *data,
							 uint32_t len) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct metal_io_region *io = NULL;
	uint32_t buff_len = 0;
	void *buffer = NULL;

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	/* Get the payload buffer. */
	buffer = rpmsg_virtio_get_tx_msg_buffer(rdev, &buff_len);
	if (!buffer) {
		return RPMSG_ERR_NO_BUFF;
	}

	/* Copy data to rpmsg buffer. */
	if (len > (int)buff_len) {
		len = buff_len;
	}

	io = rvdev->shbuf_io;
	metal_io_block_write(io, metal_io_virt_to_offset(io, buffer), data, len);

	return rpmsg_virtio_send_tx_msg_buffer(rdev, buffer, len);
}

static int rpmsg_virtio_receive(struct rpmsg_device *rdev, void *buffer,
								uint32_t len) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct rpmsg_hdr *rp_hdr = NULL;
	int status = RPMSG_SUCCESS;
	int ret_len = 0;
	uint32_t size = 0;
	uint16_t idx = 0;

	if ((!rdev)) {
		return RPMSG_ERR_PARAM;
	}

	if ((!buffer) || (len == 0)) {
		return RPMSG_ERR_NO_BUFF;
	}

	/* Get the associated remote device for channel. */
	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);

	/* Validate device state */
	status = rpmsg_virtio_get_status(rvdev);
	if (!(status & VIRTIO_CONFIG_STATUS_DRIVER_OK)) {
		return RPMSG_ERR_DEV_STATE;
	}

	/* Process the received data from remote node */
	metal_mutex_acquire(&rdev->lock);
	rp_hdr = rpmsg_virtio_get_rx_buffer(rvdev, &size, &idx);
	metal_mutex_release(&rdev->lock);
	if (!rp_hdr) {
		virtqueue_kick(rvdev->rvq);
		return RPMSG_ERR_NO_BUFF;
	}

	ret_len = metal_io_block_read(
		rvdev->shbuf_io,
		metal_io_virt_to_offset(rvdev->shbuf_io, RPMSG_LOCATE_DATA(rp_hdr)),
		buffer, rp_hdr->len > len ? len : rp_hdr->len);

	metal_mutex_acquire(&rdev->lock);
	rpmsg_virtio_return_buffer(rvdev, rp_hdr, size, idx);
	metal_mutex_release(&rdev->lock);

	return ret_len;
}

int rpmsg_virtio_init(struct rpmsg_virtio_device *rvdev,
					  struct virtio_device *vdev,
					  struct metal_io_region *shm_io,
					  struct rpmsg_virtio_shm_pool *shpool,
					  const struct rpmsg_virtio_config *config) {
	struct rpmsg_device *rdev;
	const char *vq_names[RPMSG_NUM_VRINGS];
	vq_callback callback[RPMSG_NUM_VRINGS];
	int status;
	unsigned int i, role;

	if (!rvdev || !vdev || !shm_io || !shpool || !config) {
		return RPMSG_ERR_PARAM;
	}

	/* Initializes the RPMSG virtual device */
	rdev = &rvdev->rdev;
	rdev->ops = &rpmsg_virtio_ops;
	rdev->rpmsg_msg_cb = NULL;
	metal_mutex_init(&rdev->lock);

	rvdev->vdev = vdev;
	rvdev->priv = NULL;
	vdev->priv = rvdev;
	rvdev->config = *config;
	role = rpmsg_virtio_get_role(rvdev);

	if (role == RPMSG_HOST) {
		if (!shpool->size) {
			return RPMSG_ERR_NO_BUFF;
		}
		vq_names[0] = "rx_vq";
		vq_names[1] = "tx_vq";
		callback[0] = rpmsg_virtio_rx_callback;
		callback[1] = rpmsg_virtio_tx_callback;
		rvdev->rvq = vdev->vrings_info[0].vq;
		rvdev->svq = vdev->vrings_info[1].vq;
	}

	if (role == RPMSG_REMOTE) {
		vq_names[0] = "tx_vq";
		vq_names[1] = "rx_vq";
		callback[0] = rpmsg_virtio_tx_callback;
		callback[1] = rpmsg_virtio_rx_callback;
		rvdev->rvq = vdev->vrings_info[1].vq;
		rvdev->svq = vdev->vrings_info[0].vq;
	}

	rvdev->shbuf_io = shm_io;
	rvdev->shpool = shpool;

	/* Create virtqueues for remote device */
	status = rpmsg_virtio_create_virtqueues(rvdev, 0, RPMSG_NUM_VRINGS,
											vq_names, callback);
	if (status != RPMSG_SUCCESS) {
		return status;
	}

	virtqueue_disable_cb(rvdev->svq);

	/* TODO: can have a virtio function to set the shared memory I/O */
	for (i = 0; i < RPMSG_NUM_VRINGS; i++) {
		struct virtqueue *vq;

		vq = vdev->vrings_info[i].vq;
		vq->shm_io = shm_io;
	}

	if (role == RPMSG_HOST) {
		struct virtqueue_buf vqbuf;
		unsigned int idx;
		void *buffer;

		vqbuf.len = rvdev->config.r2h_buf_size;
		for (idx = 0; idx < rvdev->rvq->vq_nentries; idx++) {
			/* Initialize TX virtqueue buffers for remote device */
			buffer = rpmsg_virtio_shm_pool_get_buffer(
				shpool, rvdev->config.r2h_buf_size);

			if (!buffer) {
				return RPMSG_ERR_NO_BUFF;
			}

			vqbuf.buf = buffer;

			metal_io_block_set(shm_io, metal_io_virt_to_offset(shm_io, buffer),
							   0x00, rvdev->config.r2h_buf_size);
			status = virtqueue_add_buffer(rvdev->rvq, &vqbuf, 0, 1, buffer);

			if (status != RPMSG_SUCCESS) {
				return status;
			}
		}

		rpmsg_virtio_set_status(rvdev, VIRTIO_CONFIG_STATUS_DRIVER_OK);
	}

	return status;
}
