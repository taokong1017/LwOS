#ifndef __VIRTIO_RING_H__
#define __VIRTIO_RING_H__

#include <types.h>
#include <compiler.h>

/* This marks a buffer as continuing via the next field. */
#define VRING_DESC_F_NEXT 1

/* The Host uses this in used->flags to advise the Guest: don't kick me
 * when you add a buffer.  It's unreliable, so it's simply an
 * optimization.  Guest will still kick if it's out of buffers.
 */
#define VRING_USED_F_NO_NOTIFY 1
/* The Guest uses this in avail->flags to advise the Host: don't
 * interrupt me when you consume a buffer.  It's unreliable, so it's
 * simply an optimization.
 */
#define VRING_AVAIL_F_NO_INTERRUPT 1

/**
 * @brief VirtIO ring descriptors.
 *
 * The descriptor table refers to the buffers the driver is using for the
 * device. addr is a physical address, and the buffers can be chained via \ref
 * next. Each descriptor describes a buffer which is read-only for the device
 * (“device-readable”) or write-only for the device (“device-writable”), but a
 * chain of descriptors can contain both device-readable and device-writable
 * buffers.
 */
struct vring_desc {
	/* Address (guest-physical) */
	uint64_t addr;

	/* Length */
	uint32_t len;

	/* Flags relevant to the descriptors */
	uint16_t flags;

	/* We chain unused descriptors via this, too */
	uint16_t next;
} PACKED;

/**
 * @brief Used to offer buffers to the device.
 *
 * Each ring entry refers to the head of a descriptor chain. It is only
 * written by the driver and read by the device.
 */
struct vring_avail {
	/* Flag which determines whether device notifications are required */
	uint16_t flags;

	/**
	 * Indicates where the driver puts the next descriptor entry in the
	 * ring (modulo the queue size)
	 */
	uint16_t idx;

	/* The ring of descriptors */
	uint16_t ring[0];
} PACKED;

/* uint32_t is used here for ids for padding reasons. */
struct vring_used_elem {
	/* Index of start of used descriptor chain. */
	uint32_t id;

	/* Total length of the descriptor chain which was written to. */
	uint32_t len;
} PACKED;

/**
 * @brief The device returns buffers to this structure when done with them
 *
 * The structure is only written to by the device, and read by the driver.
 */
struct vring_used {
	/* Flag which determines whether device notifications are required */
	uint16_t flags;

	/**
	 * Indicates where the driver puts the next descriptor entry in the
	 * ring (modulo the queue size)
	 */
	uint16_t idx;

	/* The ring of descriptors */
	struct vring_used_elem ring[0];
} PACKED;

/**
 * @brief The virtqueue layout structure
 *
 * Each virtqueue consists of; descriptor table, available ring, used ring,
 * where each part is physically contiguous in guest memory.
 *
 * When the driver wants to send a buffer to the device, it fills in a slot in
 * the descriptor table (or chains several together), and writes the descriptor
 * index into the available ring. It then notifies the device. When the device
 * has finished a buffer, it writes the descriptor index into the used ring,
 * and sends an interrupt.
 *
 * The standard layout for the ring is a continuous chunk of memory which
 * looks like this.
 *
 * struct vring {
 *      struct vring_desc desc[num];
 *
 *      __u16 avail_flags;
 *      __u16 avail_idx;
 *      __u16 available[num];
 *      __u16 used_event_idx;
 *
 *      char pad[];
 *
 *      __u16 used_flags;
 *      __u16 used_idx;
 *      struct vring_used_elem used[num];
 *      __u16 avail_event_idx;
 * };
 *
 * NOTE: for VirtIO PCI, align is 4096.
 */
struct vring {
	/**
	 * The maximum number of buffer descriptors in the virtqueue.
	 * The value is always a power of 2.
	 */
	unsigned int num;

	/* The actual buffer descriptors, 16 bytes each */
	struct vring_desc *desc;

	/* A ring of available descriptor heads with free-running index */
	struct vring_avail *avail;

	/* A ring of used descriptor heads with free-running index */
	struct vring_used *used;
};

static inline int vring_size(unsigned int num, unsigned long align) {
	int size;

	size = num * sizeof(struct vring_desc);
	size += sizeof(struct vring_avail) + (num * sizeof(uint16_t)) +
			sizeof(uint16_t);
	size = (size + align - 1) & ~(align - 1);
	size += sizeof(struct vring_used) + (num * sizeof(struct vring_used_elem)) +
			sizeof(uint16_t);
	size = (size + align - 1) & ~(align - 1);

	return size;
}

static inline void vring_init(struct vring *vr, unsigned int num, uint8_t *p,
							  unsigned long align) {
	vr->num = num;
	vr->desc = (struct vring_desc *)p;
	vr->avail = (struct vring_avail *)(p + num * sizeof(struct vring_desc));
	vr->used = (struct vring_used *)(((unsigned long)&vr->avail->ring[num] +
									  sizeof(uint16_t) + align - 1) &
									 ~(align - 1));
}

#endif