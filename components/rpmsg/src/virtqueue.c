#include <virtio.h>
#include <virtqueue.h>
#include <metal_atomic.h>
#include <metal_alloc.h>
#include <log.h>

#define VIRTQUEUE_TAG "Virtqueue"

#define VQ_PARAM_CHK(condition, status_var, status_err)                        \
	do {                                                                       \
		if (((status_var) == 0) && (condition)) {                              \
			status_var = status_err;                                           \
		}                                                                      \
	} while (0)

/* Prototype for internal functions. */
static void vq_ring_init(struct virtqueue *, void *, int);
static void vq_ring_update_avail(struct virtqueue *, uint16_t);
static uint16_t vq_ring_add_buffer(struct virtqueue *, struct vring_desc *,
								   uint16_t, struct virtqueue_buf *, int, int);
static void vq_ring_free_chain(struct virtqueue *, uint16_t);
static int vq_ring_must_notify(struct virtqueue *vq);
static void vq_ring_notify(struct virtqueue *vq);

/* Default implementation of P2V based on libmetal */
static inline void *virtqueue_phys_to_virt(struct virtqueue *vq,
										   metal_phys_addr_t phys) {
	struct metal_io_region *io = vq->shm_io;

	return metal_io_phys_to_virt(io, phys);
}

/* Default implementation of V2P based on libmetal */
static inline metal_phys_addr_t virtqueue_virt_to_phys(struct virtqueue *vq,
													   void *buf) {
	struct metal_io_region *io = vq->shm_io;

	return metal_io_virt_to_phys(io, buf);
}

int virtqueue_create(struct virtio_device *virt_dev, uint16_t id,
					 const char *name, struct vring_alloc_info *ring,
					 void (*callback)(struct virtqueue *vq),
					 void (*notify)(struct virtqueue *vq),
					 struct virtqueue *vq) {
	int status = VQUEUE_SUCCESS;

	VQ_PARAM_CHK(ring == NULL, status, ERROR_VQUEUE_INVLD_PARAM);
	VQ_PARAM_CHK(ring->num_descs == 0, status, ERROR_VQUEUE_INVLD_PARAM);
	VQ_PARAM_CHK(ring->num_descs & (ring->num_descs - 1), status,
				 ERROR_VRING_ALIGN);
	VQ_PARAM_CHK(vq == NULL, status, ERROR_NO_MEM);

	if (status == VQUEUE_SUCCESS) {
		vq->vq_dev = virt_dev;
		vq->vq_name = name;
		vq->vq_queue_index = id;
		vq->vq_nentries = ring->num_descs;
		vq->vq_free_cnt = vq->vq_nentries;
		vq->callback = callback;
		vq->notify = notify;

		/* Initialize vring control block in virtqueue. */
		vq_ring_init(vq, ring->vaddr, ring->align);
	}

	/*
	 * CACHE: nothing to be done here. Only desc.next is setup at this
	 * stage but that is only written by driver, so no need to flush it.
	 */

	return status;
}

int virtqueue_add_buffer(struct virtqueue *vq, struct virtqueue_buf *buf_list,
						 int readable, int writable, void *cookie) {
	struct vq_desc_extra *dxp = NULL;
	int status = VQUEUE_SUCCESS;
	uint16_t head_idx;
	uint16_t idx;
	int needed;

	needed = readable + writable;

	VQ_PARAM_CHK(vq == NULL, status, ERROR_VQUEUE_INVLD_PARAM);
	VQ_PARAM_CHK(needed < 1, status, ERROR_VQUEUE_INVLD_PARAM);
	VQ_PARAM_CHK(vq->vq_free_cnt < needed, status, ERROR_VRING_FULL);
	log_debug(VIRTQUEUE_TAG,
			  "virtqueue_add_buffer vq_free_cnt = %d, needed = %d\n",
			  vq->vq_free_cnt, needed);
	if (status == VQUEUE_SUCCESS) {
		head_idx = vq->vq_desc_head_idx;
		dxp = &vq->vq_descx[head_idx];

		dxp->cookie = cookie;
		dxp->ndescs = needed;

		/* Enqueue buffer onto the ring. */
		idx = vq_ring_add_buffer(vq, vq->vq_ring.desc, head_idx, buf_list,
								 readable, writable);

		vq->vq_desc_head_idx = idx;
		vq->vq_free_cnt -= needed;

		/*
		 * Update vring_avail control block fields so that other
		 * side can get buffer using it.
		 */
		vq_ring_update_avail(vq, head_idx);
	}

	return status;
}

void *virtqueue_get_buffer(struct virtqueue *vq, uint32_t *len, uint16_t *idx) {
	struct vring_used_elem *uep;
	void *cookie;
	uint16_t used_idx, desc_idx;

	VRING_INVALIDATE(&vq->vq_ring.used->idx, sizeof(vq->vq_ring.used->idx));
	if (!vq || vq->vq_used_cons_idx == vq->vq_ring.used->idx) {
		return NULL;
	}
	log_debug(
		VIRTQUEUE_TAG,
		"virtqueue_get_buffer [%s] vq_used_cons_idx = %d, used->idx = %d\n",
		vq->vq_name, vq->vq_used_cons_idx, vq->vq_ring.used->idx);

	used_idx = vq->vq_used_cons_idx++ & (vq->vq_nentries - 1);
	uep = &vq->vq_ring.used->ring[used_idx];

	atomic_thread_fence();

	/* Used.ring is written by remote, invalidate it */
	VRING_INVALIDATE(&vq->vq_ring.used->ring[used_idx],
					 sizeof(vq->vq_ring.used->ring[used_idx]));

	desc_idx = (uint16_t)uep->id;
	if (len) {
		*len = uep->len;
	}

	vq_ring_free_chain(vq, desc_idx);

	cookie = vq->vq_descx[desc_idx].cookie;
	vq->vq_descx[desc_idx].cookie = NULL;

	if (idx) {
		*idx = used_idx;
	}

	return cookie;
}

uint32_t virtqueue_get_buffer_length(struct virtqueue *vq, uint16_t idx) {
	VRING_INVALIDATE(&vq->vq_ring.desc[idx].len,
					 sizeof(vq->vq_ring.desc[idx].len));
	return vq->vq_ring.desc[idx].len;
}

void *virtqueue_get_buffer_addr(struct virtqueue *vq, uint16_t idx) {
	VRING_INVALIDATE(&vq->vq_ring.desc[idx].addr,
					 sizeof(vq->vq_ring.desc[idx].addr));
	return virtqueue_phys_to_virt(vq, vq->vq_ring.desc[idx].addr);
}

void *virtqueue_get_available_buffer(struct virtqueue *vq, uint16_t *avail_idx,
									 uint32_t *len) {
	uint16_t head_idx = 0;
	void *buffer;

	atomic_thread_fence();

	/* Avail.idx is updated by driver, invalidate it */
	VRING_INVALIDATE(&vq->vq_ring.avail->idx, sizeof(vq->vq_ring.avail->idx));
	if (vq->vq_available_idx == vq->vq_ring.avail->idx) {
		return NULL;
	}

	log_debug(VIRTQUEUE_TAG,
			  "virtqueue_get_available_buffer [%s] vq_available_idx = %d, "
			  "avail->idx = %d\n",
			  vq->vq_name, vq->vq_available_idx, vq->vq_ring.avail->idx);
	head_idx = vq->vq_available_idx++ & (vq->vq_nentries - 1);

	/* Avail.ring is updated by driver, invalidate it */
	VRING_INVALIDATE(&vq->vq_ring.avail->ring[head_idx],
					 sizeof(vq->vq_ring.avail->ring[head_idx]));
	*avail_idx = vq->vq_ring.avail->ring[head_idx];

	/* Invalidate the desc entry written by driver before accessing it */
	VRING_INVALIDATE(&vq->vq_ring.desc[*avail_idx],
					 sizeof(vq->vq_ring.desc[*avail_idx]));
	buffer = virtqueue_phys_to_virt(vq, vq->vq_ring.desc[*avail_idx].addr);
	*len = vq->vq_ring.desc[*avail_idx].len;

	return buffer;
}

int virtqueue_add_consumed_buffer(struct virtqueue *vq, uint16_t head_idx,
								  uint32_t len) {
	struct vring_used_elem *used_desc = NULL;
	uint16_t used_idx;

	if (head_idx >= vq->vq_nentries) {
		return ERROR_VRING_NO_BUFF;
	}
	log_debug(VIRTQUEUE_TAG,
			  "virtqueue_add_consumed_buffer head_idx = %d, "
			  "vq->vq_nentries = %d\n",
			  head_idx, vq->vq_nentries);

	/* CACHE: used is never written by driver, so it's safe to directly access
	 * it
	 */
	used_idx = vq->vq_ring.used->idx & (vq->vq_nentries - 1);
	used_desc = &vq->vq_ring.used->ring[used_idx];
	used_desc->id = head_idx;
	used_desc->len = len;

	/* We still need to flush it because this is read by driver */
	VRING_FLUSH(&vq->vq_ring.used->ring[used_idx],
				sizeof(vq->vq_ring.used->ring[used_idx]));

	atomic_thread_fence();

	vq->vq_ring.used->idx++;
	log_debug(VIRTQUEUE_TAG, "virtqueue_add_consumed_buffer used->idx = %d\n",
			  vq->vq_ring.used->idx);
	/* Used.idx is read by driver, so we need to flush it */
	VRING_FLUSH(&vq->vq_ring.used->idx, sizeof(vq->vq_ring.used->idx));

	return VQUEUE_SUCCESS;
}

/*
 *
 * vq_ring_enable_interrupt
 *
 */
static int vq_ring_enable_interrupt(struct virtqueue *vq, uint16_t ndesc) {
	if (vq->vq_dev->role == VIRTIO_DEV_DRIVER) {
		vq->vq_ring.avail->flags &= ~VRING_AVAIL_F_NO_INTERRUPT;
		VRING_FLUSH(&vq->vq_ring.avail->flags,
					sizeof(vq->vq_ring.avail->flags));
	}

	if (vq->vq_dev->role == VIRTIO_DEV_DEVICE) {
		vq->vq_ring.used->flags &= ~VRING_USED_F_NO_NOTIFY;
		VRING_FLUSH(&vq->vq_ring.used->flags, sizeof(vq->vq_ring.used->flags));
	}

	atomic_thread_fence();
	return 0;
}

int virtqueue_enable_cb(struct virtqueue *vq) {
	return vq_ring_enable_interrupt(vq, 0);
}

void virtqueue_disable_cb(struct virtqueue *vq) {
	if (vq->vq_dev->role == VIRTIO_DEV_DRIVER) {
		vq->vq_ring.avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;
		VRING_FLUSH(&vq->vq_ring.avail->flags,
					sizeof(vq->vq_ring.avail->flags));
	}

	if (vq->vq_dev->role == VIRTIO_DEV_DEVICE) {
		vq->vq_ring.used->flags |= VRING_USED_F_NO_NOTIFY;
		VRING_FLUSH(&vq->vq_ring.used->flags, sizeof(vq->vq_ring.used->flags));
	}
}

void virtqueue_kick(struct virtqueue *vq) {
	/* Ensure updated avail->idx is visible to host. */
	atomic_thread_fence();

	if (vq_ring_must_notify(vq)) {
		vq_ring_notify(vq);
	}
}

/**************************************************************************
 *                            Helper Functions                            *
 **************************************************************************/

/*
 *
 * vq_ring_add_buffer
 *
 */
static uint16_t vq_ring_add_buffer(struct virtqueue *vq,
								   struct vring_desc *desc, uint16_t head_idx,
								   struct virtqueue_buf *buf_list, int readable,
								   int writable) {
	struct vring_desc *dp;
	int i, needed;
	uint16_t idx;

	(void)vq;

	needed = readable + writable;

	for (i = 0, idx = head_idx; i < needed; i++, idx = dp->next) {
		/* CACHE: No need to invalidate desc because it is only written by
		 * driver */
		dp = &desc[idx];
		dp->addr = virtqueue_virt_to_phys(vq, buf_list[i].buf);
		dp->len = buf_list[i].len;
		dp->flags = 0;

		if (i < needed - 1)
			dp->flags |= VRING_DESC_F_NEXT;

		/*
		 * Instead of flushing the whole desc region, we flush only the
		 * single entry hopefully saving some cycles
		 */
		VRING_FLUSH(&desc[idx], sizeof(desc[idx]));
	}

	return idx;
}

/*
 *
 * vq_ring_free_chain
 *
 */
static void vq_ring_free_chain(struct virtqueue *vq, uint16_t desc_idx) {
	struct vring_desc *dp;
	struct vq_desc_extra *dxp;
	log_debug(VIRTQUEUE_TAG,
			  "vq_ring_free_chain vq_free_cnt = %d, desc_idx = %u\n",
			  vq->vq_free_cnt, desc_idx);

	dp = &vq->vq_ring.desc[desc_idx];
	dxp = &vq->vq_descx[desc_idx];

	vq->vq_free_cnt += dxp->ndescs;
	log_debug(VIRTQUEUE_TAG,
			  "vq_ring_free_chain vq_free_cnt = %d, dxp->ndescs = %u\n",
			  vq->vq_free_cnt, dxp->ndescs);
	dxp->ndescs--;

	while (dp->flags & VRING_DESC_F_NEXT) {
		dp = &vq->vq_ring.desc[dp->next];
		dxp->ndescs--;
	}

	/*
	 * We must append the existing free chain, if any, to the end of
	 * newly freed chain. If the virtqueue was completely used, then
	 * head would be VQ_RING_DESC_CHAIN_END (ASSERTed above).
	 *
	 * CACHE: desc.next is never read by remote, no need to flush it.
	 */
	dp->next = vq->vq_desc_head_idx;
	vq->vq_desc_head_idx = desc_idx;
}

/*
 *
 * vq_ring_init
 *
 */
static void vq_ring_init(struct virtqueue *vq, void *ring_mem, int alignment) {
	struct vring *vr;
	int size;

	size = vq->vq_nentries;
	vr = &vq->vq_ring;

	vring_init(vr, size, ring_mem, alignment);

	if (vq->vq_dev->role == VIRTIO_DEV_DRIVER) {
		int i = 0;
		for (i = 0; i < size - 1; i++) {
			vr->desc[i].next = i + 1;
		}
		vr->desc[i].next = VQ_RING_DESC_CHAIN_END;
	}
}

/*
 *
 * vq_ring_update_avail
 *
 */
static void vq_ring_update_avail(struct virtqueue *vq, uint16_t desc_idx) {
	uint16_t avail_idx;

	/*
	 * Place the head of the descriptor chain into the next slot and make
	 * it usable to the host. The chain is made available now rather than
	 * deferring to virtqueue_notify() in the hopes that if the host is
	 * currently running on another CPU, we can keep it processing the new
	 * descriptor.
	 *
	 * CACHE: avail is never written by remote, so it is safe to not invalidate
	 * here
	 */
	avail_idx = vq->vq_ring.avail->idx & (vq->vq_nentries - 1);
	vq->vq_ring.avail->ring[avail_idx] = desc_idx;

	/* We still need to flush the ring */
	VRING_FLUSH(&vq->vq_ring.avail->ring[avail_idx],
				sizeof(vq->vq_ring.avail->ring[avail_idx]));

	atomic_thread_fence();

	vq->vq_ring.avail->idx++;
	log_debug(VIRTQUEUE_TAG, "vq_ring_update_avail avail->idx = %d\n",
			  vq->vq_ring.avail->idx);
	/* And the index */
	VRING_FLUSH(&vq->vq_ring.avail->idx, sizeof(vq->vq_ring.avail->idx));
}

/*
 *
 * virtqueue_interrupt
 *
 */
void virtqueue_notification(struct virtqueue *vq) {
	atomic_thread_fence();
	if (vq->callback) {
		vq->callback(vq);
	}
}

/*
 *
 * vq_ring_must_notify
 *
 */
static int vq_ring_must_notify(struct virtqueue *vq) {
	if (vq->vq_dev->role == VIRTIO_DEV_DRIVER) {
		VRING_INVALIDATE(&vq->vq_ring.used->flags,
						 sizeof(vq->vq_ring.used->flags));
		return (vq->vq_ring.used->flags & VRING_USED_F_NO_NOTIFY) == 0;
	}
	if (vq->vq_dev->role == VIRTIO_DEV_DEVICE) {
		VRING_INVALIDATE(&vq->vq_ring.avail->flags,
						 sizeof(vq->vq_ring.avail->flags));
		return (vq->vq_ring.avail->flags & VRING_AVAIL_F_NO_INTERRUPT) == 0;
	}

	return 0;
}

/*
 *
 * vq_ring_notify
 *
 */
static void vq_ring_notify(struct virtqueue *vq) {
	if (vq->notify) {
		vq->notify(vq);
	}
}
