#include <virtio.h>

int virtio_create_virtqueues(struct virtio_device *vdev, unsigned int flags,
							 unsigned int nvqs, const char *names[],
							 vq_callback callbacks[], void *callback_args[]) {
	struct virtio_vring_info *vring_info;
	struct vring_alloc_info *vring_alloc;
	unsigned int num_vrings, i;
	int ret;
	(void)flags;

	if (!vdev) {
		return -EINVAL;
	}

	num_vrings = vdev->vrings_num;
	if (nvqs > num_vrings) {
		return ERROR_VQUEUE_INVLD_PARAM;
	}

	/* Initialize virtqueue for each vring */
	for (i = 0; i < nvqs; i++) {
		vring_info = &vdev->vrings_info[i];

		vring_alloc = &vring_info->info;
		if (vdev->role == VIRTIO_DEV_DRIVER) {
			struct metal_io_region *io = vring_info->io;

			size_t offset = metal_io_virt_to_offset(io, vring_alloc->vaddr);
			metal_io_block_set(
				io, offset, 0,
				vring_size(vring_alloc->num_descs, vring_alloc->align));
		}
		ret = virtqueue_create(vdev, i, names[i], vring_alloc, callbacks[i],
							   vdev->func->notify, vring_info->vq);
		if (ret) {
			return ret;
		}
	}

	return 0;
}
