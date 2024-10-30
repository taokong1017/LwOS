#include <metal_alloc.h>
#include <rpmsg.h>
#include <string.h>
#include <rpmsg_virtio.h>
#include <log.h>

#define RPMSG_TAG "rpmsg"
#define IS_POWER_OF_TWO(n) ((n & (n - 1)) == 0)
#define rpmsg_id_to_dev(id) ((struct rpmsg_device *)id)
#define rpmsg_dev_to_id(dev) ((rpmsg_id_t)dev)
#define rpmsg_buffer_size(msg_len) (msg_len + sizeof(struct rpmsg_hdr))
#define rpmsg_sync_type_is_invalid(type)                                       \
	((type < SHM_NO_SYNC || type > SHM_SYNC_OUTER) ? true : false)
#define rpmsg_buf_info_is_invalid(buf_info)                                    \
	((!buf_info.h2r_buf_num || !buf_info.h2r_buf_size) &&                      \
	 (!buf_info.h2r_buf_num || !buf_info.h2r_buf_size))

#define RPMSG_MAGIC_LEN 8
#define RPMSG_STATUS_MAGIC "RPMSG"
#define RPMSG_VRING_ALIGN 8

/**
 *  The shared memory layout as follows:
 * |                  |              |              |                 |
 * |<- rpmsg status ->|<- vring[0] ->|<- vring[1] ->|<- shared pool ->|
 * |                  |              |              |                 |
 * These macros from `rpmsg_status_size` to `rpmsg_min_buf_size` use the above
 * memory layout to caculate starting address and size.
 */
#define rpmsg_status_size (sizeof(struct rpmsg_status))
#define rpmsg_status_addr(shbuf) ((struct rpmsg_status *)shbuf)
#define rpmsg_status_get(shbuf) (rpmsg_status_addr(shbuf)->status)
#define rpmsg_status_set(shbuf, status)                                        \
	(rpmsg_status_addr(shbuf)->status = status)
#define rpmsg_magic_get(shbuf) (rpmsg_status_addr(shbuf)->magic)

#define rpmsg_vring0_addr(shbuf) (shbuf + rpmsg_status_size)
#define rpmsg_vring_size(desc_num, align) (vring_size(desc_num, align))
#define rpmsg_vring1_addr(shbuf, desc0_num, align)                             \
	(rpmsg_vring0_addr(shbuf) + rpmsg_vring_size(desc0_num, align))

#define rpmsg_pool_addr(shbuf, desc0_num, desc1_num, align)                    \
	(rpmsg_vring1_addr(shbuf, desc0_num, align) +                              \
	 rpmsg_vring_size(desc1_num, align))
#define rpmsg_pool_size(desc0_num, buf0_size, desc1_num, buf1_size)            \
	(desc0_num * rpmsg_buffer_size(buf0_size) +                                \
	 desc1_num * rpmsg_buffer_size(buf1_size))

#define rpmsg_min_buf_size(desc0_num, buf0_size, desc1_num, buf1_size, align)  \
	(rpmsg_status_size + rpmsg_vring_size(desc0_num, align) +                  \
	 rpmsg_vring_size(desc1_num, align) +                                      \
	 rpmsg_pool_size(desc0_num, buf0_size, desc1_num, buf1_size))

/**
 * @struct rpmsg_status
 * @brief Represents the status of an RPMSG communication channel.
 *
 * This structure contains information about the status of an RPMSG
 * communication channel, including a magic number to identify the RPMSG
 * protocol, the current status of the channel, and a reserved field.
 */
struct rpmsg_status {
	uint32_t magic[RPMSG_MAGIC_LEN];
	uint32_t status;
	uint32_t reserved;
};

int32_t rpmsg_send(rpmsg_id_t id, void *data, uint32_t len) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	struct rpmsg_ops *ops = NULL;
	if (!rdev || !data || !len) {
		log_err(RPMSG_TAG,
				"fails to send rpmsg because of empty input paramer.\n");
		return RPMSG_ERR_PARAM;
	}

	ops = rdev->ops;
	if (ops) {
		return ops->send(rdev, data, len);
	} else {
		return RPMSG_ERR_INIT;
	}
}

int32_t rpmsg_receive(rpmsg_id_t id, void *buffer, uint32_t len) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	struct rpmsg_ops *ops = NULL;

	if (!rdev || !buffer || !len) {
		log_err(RPMSG_TAG,
				"fails to receive rpmsg because of empty input paramer.\n");
		return RPMSG_ERR_PARAM;
	}

	ops = rdev->ops;
	if (ops) {
		return ops->receive(rdev, buffer, len);
	} else {
		return RPMSG_ERR_INIT;
	}
}

int32_t rpmsg_msg_cb_register(rpmsg_id_t id, rpmsg_msg_cb cb) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	if (!rdev || !cb) {
		log_err(RPMSG_TAG, "fails to register message callback because of "
						   "empty input paramer.\n");
		return RPMSG_ERR_PARAM;
	}

	rdev->rpmsg_msg_cb = cb;

	return RPMSG_SUCCESS;
}

void rpmsg_receive_handler(rpmsg_id_t id) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	struct rpmsg_virtio_device *rvdev = NULL;

	if (!rdev) {
		log_err(
			RPMSG_TAG,
			"fails to start rpmsg handler because of empty input paramer.\n");
		return;
	}

	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);
	virtqueue_notification(rvdev->rvq);
}

int32_t rpmsg_notify_register(rpmsg_id_t id, rpmsg_notify notify, void *arg0,
							  void *arg1) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	struct rpmsg_virtio_device *rvdev = NULL;
	struct rpmsg_notify_info *notify_info = NULL;

	if (!rdev || !notify) {
		log_err(RPMSG_TAG, "fails to register notify callbace because of "
						   "empty input paramer.\n");
		return RPMSG_ERR_PARAM;
	}

	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);
	if (!rvdev || rvdev->priv) {
		return RPMSG_ERR_DEV_STATE;
	}

	notify_info = metal_allocate_memory(sizeof(struct rpmsg_notify_info));
	if (!notify_info) {
		log_err(
			RPMSG_TAG,
			"fails to malloc memory for notification regiser information.\n");
		return RPMSG_ERR_NO_MEM;
	}
	notify_info->notify = notify;
	notify_info->arg0 = arg0;
	notify_info->arg1 = arg1;
	rvdev->priv = notify_info;

	/* Enable callback for virtqueue */
	virtqueue_enable_cb(rvdev->svq);

	return RPMSG_SUCCESS;
}

static uint8_t virtio_get_status(struct virtio_device *vdev) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct metal_io_region *shbuf_io = NULL;
	void *shbuf = NULL;

	if (!vdev || !vdev->priv) {
		log_err(
			RPMSG_TAG,
			"fails to get rpmsg status because of empty input parameter.\n");
		return VIRTIO_CONFIG_STATUS_FAILED;
	}

	rvdev = (struct rpmsg_virtio_device *)vdev->priv;
	shbuf_io = rvdev->shbuf_io;
	shbuf = shbuf_io->virt;

	atomic_thread_fence();
	if (memcmp(rpmsg_magic_get(shbuf), RPMSG_STATUS_MAGIC,
			   strlen(RPMSG_STATUS_MAGIC))) {
		return VIRTIO_CONFIG_STATUS_FAILED;
	}

	return rpmsg_status_get(shbuf);
}

static void virtio_set_status(struct virtio_device *vdev, uint8_t status) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct metal_io_region *shbuf_io = NULL;
	void *shbuf = NULL;

	if (!vdev || !vdev->priv) {
		log_err(
			RPMSG_TAG,
			"fails to set rpmsg status because of empty input parameter.\n");
		return;
	}

	rvdev = (struct rpmsg_virtio_device *)vdev->priv;
	shbuf_io = rvdev->shbuf_io;
	shbuf = shbuf_io->virt;

	memcpy(rpmsg_magic_get(shbuf), RPMSG_STATUS_MAGIC,
		   strlen(RPMSG_STATUS_MAGIC));
	rpmsg_status_set(shbuf, status);
	atomic_thread_fence();
}

static void virtio_virtqueue_notify(struct virtqueue *vq) {
	struct rpmsg_virtio_device *rvdev = NULL;
	struct virtio_device *vdev = NULL;
	struct rpmsg_notify_info *notify_info = NULL;

	if (!vq || !vq->vq_dev) {
		log_err(
			RPMSG_TAG,
			"fails to notify rpmsg remote because of empty input parameter.\n");
		return;
	}

	vdev = vq->vq_dev;
	rvdev = (struct rpmsg_virtio_device *)vdev->priv;
	if (!rvdev || !rvdev->priv) {
		return;
	}

	notify_info = (struct rpmsg_notify_info *)rvdev->priv;
	if (!notify_info) {
		return;
	}
	notify_info->notify(notify_info->arg0, notify_info->arg1);
}

static const struct virtio_dispatch virtio_dispatch = {
	.get_status = virtio_get_status,
	.set_status = virtio_set_status,
	.notify = virtio_virtqueue_notify,
};

static int32_t virtio_vring_init(struct virtio_vring_info *vrings_info,
								 int32_t role, uint32_t num_desc, void *vaddr,
								 uint32_t align, struct metal_io_region *io) {
	struct virtqueue *vq = NULL;
	struct vring_alloc_info *alloc_info = NULL;

	vq = virtqueue_allocate(num_desc);
	if (!vq) {
		log_err(RPMSG_TAG, "fails to malloc memory for virtqueue.\n");
		return RPMSG_ERR_NO_MEM;
	}

	vrings_info->vq = vq;
	vrings_info->io = io;
	alloc_info = &vrings_info->info;
	alloc_info->align = align;
	alloc_info->num_descs = num_desc;
	alloc_info->vaddr = vaddr;

	return RPMSG_SUCCESS;
}

rpmsg_id_t rpmsg_init(int32_t role, struct shm_mem_info shm,
					  struct rpmsg_buf_info buf_info) {
	struct virtio_device *vdev = NULL;
	struct rpmsg_virtio_device *rvdev = NULL;
	struct metal_io_region *shm_io = NULL;
	metal_phys_addr_t *physmap = NULL;
	struct rpmsg_virtio_shm_pool *shpool = NULL;
	struct virtio_vring_info *vrings_info = NULL;
	struct rpmsg_virtio_config config = {
		.h2r_buf_size = rpmsg_buffer_size(buf_info.h2r_buf_size),
		.r2h_buf_size = rpmsg_buffer_size(buf_info.r2h_buf_size),
	};
	int32_t ret = 0;
	uint32_t min_size = 0;

	/* Check input parameters */
	if ((role != RPMSG_REMOTE) && (role != RPMSG_HOST)) {
		log_err(RPMSG_TAG, "invalid role[%d].\n", role);
		return INVALID_RPMSG_ID;
	}

	if (!shm.phys || !shm.virt || rpmsg_sync_type_is_invalid(shm.sync_type)) {
		log_err(RPMSG_TAG,
				"invalid physical address[0x%x], virtual address[0x%x], or "
				"sync type[%d].\n",
				shm.phys, shm.virt, shm.sync_type);
		return INVALID_RPMSG_ID;
	}

	min_size = rpmsg_min_buf_size(buf_info.r2h_buf_num, buf_info.r2h_buf_size,
								  buf_info.h2r_buf_num, buf_info.h2r_buf_size,
								  RPMSG_VRING_ALIGN);
	if (shm.size < min_size) {
		log_err(RPMSG_TAG, "invalid memory size[%u], is less than [%u].\n",
				shm.size, min_size);
		return INVALID_RPMSG_ID;
	}

	if ((!IS_POWER_OF_TWO(buf_info.r2h_buf_num)) ||
		(!IS_POWER_OF_TWO(buf_info.h2r_buf_num))) {
		log_err(
			RPMSG_TAG,
			"the r2h_buf_num %d and h2r_buf_num %d should be power of two.\n",
			buf_info.r2h_buf_num, buf_info.h2r_buf_num);
		return INVALID_RPMSG_ID;
	}

	if (rpmsg_buf_info_is_invalid(buf_info)) {
		log_err(RPMSG_TAG,
				"the buffer size and buffer number are both zeros.\n");
		return INVALID_RPMSG_ID;
	}

	/* Set the memory barrier type */
	atomic_type_set(shm.sync_type);

	/* Initialize shared memory IO */
	shm_io = (struct metal_io_region *)metal_allocate_memory(
		sizeof(struct metal_io_region));
	physmap =
		(metal_phys_addr_t *)metal_allocate_memory(sizeof(metal_phys_addr_t));
	if (!shm_io || !physmap) {
		log_err(RPMSG_TAG, "fails to malloc memory for shared IO.\n");
		goto err0;
	}
	physmap[0] = shm.phys;
	metal_io_init(
		shm_io, shm.virt, physmap, shm.size,
		-1 /* the input shared memory in the same memory mapped block */);

	/* Initialize virtio device */
	vdev = (struct virtio_device *)metal_allocate_memory(
		sizeof(struct virtio_device));
	vrings_info = (struct virtio_vring_info *)metal_allocate_memory(
		RPMSG_NUM_VRINGS * sizeof(struct virtio_vring_info));
	if (!vdev || !vrings_info) {
		log_err(RPMSG_TAG, "fails to malloc memory for virtual device.\n");
		goto err1;
	}
	vdev->role = role;
	vdev->func = &virtio_dispatch;
	vdev->vrings_num = RPMSG_NUM_VRINGS;
	vdev->priv = NULL;
	vdev->vrings_info = vrings_info;

	/* Initialize Vrings */
	ret = virtio_vring_init(&vrings_info[0], role, buf_info.r2h_buf_num,
							rpmsg_vring0_addr(shm_io->virt), RPMSG_VRING_ALIGN,
							shm_io);
	if (ret) {
		goto err2;
	}

	ret =
		virtio_vring_init(&vrings_info[1], role, buf_info.h2r_buf_num,
						  rpmsg_vring1_addr(shm_io->virt, buf_info.r2h_buf_num,
											RPMSG_VRING_ALIGN),
						  RPMSG_VRING_ALIGN, shm_io);
	if (ret) {
		goto err3;
	}

	/* Initialize shared pool */
	shpool = (struct rpmsg_virtio_shm_pool *)metal_allocate_memory(
		sizeof(struct rpmsg_virtio_shm_pool));
	if (!shpool) {
		log_err(RPMSG_TAG, "fails to malloc memory for shared memory pool.\n");
		goto err4;
	}
	rpmsg_virtio_init_shm_pool(
		shpool,
		rpmsg_pool_addr(shm_io->virt, buf_info.r2h_buf_num,
						buf_info.h2r_buf_num, RPMSG_VRING_ALIGN),
		rpmsg_pool_size(buf_info.r2h_buf_num, buf_info.r2h_buf_size,
						buf_info.h2r_buf_num, buf_info.h2r_buf_size));

	/* Initialize RPMsg virtio device */
	rvdev = (struct rpmsg_virtio_device *)metal_allocate_memory(
		sizeof(struct rpmsg_virtio_device));
	if (!rvdev) {
		log_err(RPMSG_TAG,
				"fails to malloc memory for rpmsg virtual device.\n");
		goto err5;
	}
	rpmsg_virtio_init(rvdev, vdev, shm_io, shpool, &config);

	return rpmsg_dev_to_id(rvdev);

err5:
	metal_free_memory(rvdev);
err4:
	metal_free_memory(shpool);
err3:
	metal_free_memory(vrings_info[1].vq);
err2:
	metal_free_memory(vrings_info[0].vq);
err1:
	metal_free_memory(vdev);
	metal_free_memory(vrings_info);
err0:
	metal_free_memory(shm_io->physmap);
	metal_free_memory(shm_io);

	return INVALID_RPMSG_ID;
}

void rpmsg_deinit(rpmsg_id_t id) {
	struct rpmsg_device *rdev = rpmsg_id_to_dev(id);
	struct rpmsg_virtio_device *rvdev = NULL;

	if (!rdev) {
		return;
	}

	rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev);
	if (!rvdev) {
		return;
	}

	virtio_set_status(rvdev->vdev, VIRTIO_CONFIG_STATUS_RESET);
	metal_free_memory(rvdev->priv);
	metal_free_memory(rvdev->shpool);
	metal_free_memory(rvdev->shbuf_io->physmap);
	metal_free_memory(rvdev->shbuf_io);
	metal_free_memory(rvdev->rvq);
	metal_free_memory(rvdev->svq);
	metal_free_memory(rvdev->vdev->vrings_info);
	metal_free_memory(rvdev->vdev);
	metal_mutex_deinit(&rdev->lock);
	metal_free_memory(rvdev);

	return;
}