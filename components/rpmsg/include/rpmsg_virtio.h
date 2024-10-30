#ifndef __RPMSG_VIRTIO_H__
#define __RPMSG_VIRTIO_H__

#include <metal_io.h>
#include <metal_cache.h>
#include <virtio.h>

#define RPMSG_NUM_VRINGS 2
#define BUFFER_FLUSH(x, s) metal_cache_flush(x, s)
#define BUFFER_INVALIDATE(x, s) metal_cache_invalidate(x, s)

/* @brief Shared memory pool used for RPMsg buffers */
struct rpmsg_virtio_shm_pool {
	/* Base address of the memory pool */
	void *base;

	/* Available memory size */
	size_t avail;

	/* Total pool size */
	size_t size;
};

/**
 * @brief Configuration of RPMsg device based on virtio
 *
 * This structure is used by the RPMsg virtio host to configure the virtiio
 * layer.
 */
struct rpmsg_virtio_config {
	/* The size of the buffer used to send data from host to remote */
	uint32_t h2r_buf_size;

	/* The size of the buffer used to send data from remote to host */
	uint32_t r2h_buf_size;
};

/* @brief Representation of a RPMsg device based on virtio */
struct rpmsg_virtio_device {
	/* RPMsg device */
	struct rpmsg_device rdev;

	/* Pointer to the virtio device */
	struct virtio_device *vdev;

	/* Structure containing virtio configuration */
	struct rpmsg_virtio_config config;

	/* Pointer to receive virtqueue */
	struct virtqueue *rvq;

	/* Pointer to send virtqueue */
	struct virtqueue *svq;

	/* Pointer to the shared buffer I/O region */
	struct metal_io_region *shbuf_io;

	/* Pointer to the shared buffers pool */
	struct rpmsg_virtio_shm_pool *shpool;

	/*  Pointer to private data */
	void *priv;
};

static inline unsigned int
rpmsg_virtio_get_role(struct rpmsg_virtio_device *rvdev) {
	return rvdev->vdev->role;
}

static inline void rpmsg_virtio_set_status(struct rpmsg_virtio_device *rvdev,
										   uint8_t status) {
	rvdev->vdev->func->set_status(rvdev->vdev, status);
}

static inline uint8_t
rpmsg_virtio_get_status(struct rpmsg_virtio_device *rvdev) {
	return rvdev->vdev->func->get_status(rvdev->vdev);
}

static inline int
rpmsg_virtio_create_virtqueues(struct rpmsg_virtio_device *rvdev, int flags,
							   unsigned int nvqs, const char *names[],
							   vq_callback *callbacks) {
	return virtio_create_virtqueues(rvdev->vdev, flags, nvqs, names, callbacks,
									NULL);
}

/**
 * @brief Initialize rpmsg virtio device
 *
 * Host side:
 * Initialize Msg virtio queues and shared buffers, the address of shm can be
 * ANY. In this case, function will get shared memory from system shared memory
 * pools.
 * @param rvdev		Pointer to the rpmsg virtio device
 * @param vdev		Pointer to the virtio device
 * @param shm_io	Pointer to the share memory I/O region.
 * @param shpool	Pointer to shared memory pool.
 * @param config	Pointer to buffer configure.
 *
 * @return Status of function execution
 */
int rpmsg_virtio_init(struct rpmsg_virtio_device *rvdev,
					  struct virtio_device *vdev,
					  struct metal_io_region *shm_io,
					  struct rpmsg_virtio_shm_pool *shpool,
					  const struct rpmsg_virtio_config *config);

/**
 * @brief Deinitialize rpmsg virtio device
 *
 * @param rvdev	Pointer to the rpmsg virtio device
 */
void rpmsg_deinit_vdev(struct rpmsg_virtio_device *rvdev);

/**
 * @brief Initialize default shared buffers pool
 *
 * RPMsg virtio has default shared buffers pool implementation.
 * The memory assigned to this pool will be dedicated to the RPMsg
 * virtio. This function has to be called before calling rpmsg_init_vdev,
 * to initialize the rpmsg_virtio_shm_pool structure.
 *
 * @param shpool	Pointer to the shared buffers pool structure
 * @param shbuf		Pointer to the beginning of shared buffers
 * @param size		Shared buffers total size
 */
void rpmsg_virtio_init_shm_pool(struct rpmsg_virtio_shm_pool *shpool,
								void *shbuf, size_t size);

/**
 * @brief Get buffer in the shared memory pool
 *
 * RPMsg virtio has default shared buffers pool implementation.
 * The memory assigned to this pool will be dedicated to the RPMsg
 * virtio. If you prefer to have other shared buffers allocation,
 * you can implement your rpmsg_virtio_shm_pool_get_buffer function.
 *
 * @param shpool	Pointer to the shared buffers pool
 * @param size		Shared buffers total size
 *
 * @return Buffer pointer if free buffer is available, NULL otherwise.
 */
void *rpmsg_virtio_shm_pool_get_buffer(struct rpmsg_virtio_shm_pool *shpool,
									   size_t size);

#endif
