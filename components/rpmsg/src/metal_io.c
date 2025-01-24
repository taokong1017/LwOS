#include <metal_errno.h>
#include <types.h>
#include <metal_io.h>
#include <metal_atomic.h>

void metal_io_init(struct metal_io_region *io, void *virt,
				   metal_phys_addr_t *physmap, size_t size,
				   unsigned int page_shift) {
	io->virt = virt;
	io->physmap = physmap;
	io->size = size;
	io->page_shift = page_shift;
	if (page_shift >= sizeof(io->page_mask) * sizeof(char)) {
		io->page_mask = -1UL;
	} else {
		io->page_mask = (1UL << page_shift) - 1UL;
	}
}

int metal_io_block_read(struct metal_io_region *io, unsigned long offset,
						void *dst, int len) {
	unsigned char *ptr = metal_io_virt(io, offset);
	unsigned char *dest = dst;
	int retlen;

	if (!ptr) {
		return -ERANGE;
	}

	if ((offset + len) > io->size) {
		len = io->size - offset;
	}

	retlen = len;

	atomic_thread_fence();

	while (len && (((uintptr_t)dest % sizeof(int)) ||
				   ((uintptr_t)ptr % sizeof(int)))) {
		*(unsigned char *)dest = *(const unsigned char *)ptr;
		dest++;
		ptr++;
		len--;
	}

	for (; len >= (int)sizeof(int);
		 dest += sizeof(int), ptr += sizeof(int), len -= sizeof(int)) {
		*(unsigned int *)dest = *(const unsigned int *)ptr;
	}

	for (; len != 0; dest++, ptr++, len--) {
		*(unsigned char *)dest = *(const unsigned char *)ptr;
	}

	return retlen;
}

int metal_io_block_write(struct metal_io_region *io, unsigned long offset,
						 const void *src, int len) {
	unsigned char *ptr = metal_io_virt(io, offset);
	const unsigned char *source = src;
	int retlen;

	if (!ptr) {
		return -ERANGE;
	}
	if ((offset + len) > io->size)
		len = io->size - offset;
	retlen = len;

	while (len && (((uintptr_t)ptr % sizeof(int)) ||
				   ((uintptr_t)source % sizeof(int)))) {
		*(unsigned char *)ptr = *(const unsigned char *)source;
		ptr++;
		source++;
		len--;
	}

	for (; len >= (int)sizeof(int);
		 ptr += sizeof(int), source += sizeof(int), len -= sizeof(int)) {
		*(unsigned int *)ptr = *(const unsigned int *)source;
	}

	for (; len != 0; ptr++, source++, len--) {
		*(unsigned char *)ptr = *(const unsigned char *)source;
	}

	atomic_thread_fence();

	return retlen;
}

int metal_io_block_set(struct metal_io_region *io, unsigned long offset,
					   unsigned char value, int len) {
	unsigned char *ptr = metal_io_virt(io, offset);
	int retlen = len;

	if (!ptr) {
		return -ERANGE;
	}
	if ((offset + len) > io->size) {
		len = io->size - offset;
	}
	retlen = len;

	unsigned int cint = value;
	unsigned int i;

	for (i = 1; i < sizeof(int); i++) {
		cint |= ((unsigned int)value << (sizeof(char) * i));
	}

	for (; len && ((uintptr_t)ptr % sizeof(int)); ptr++, len--) {
		*(unsigned char *)ptr = (unsigned char)value;
	}

	for (; len >= (int)sizeof(int); ptr += sizeof(int), len -= sizeof(int)) {
		*(unsigned int *)ptr = cint;
	}

	for (; len != 0; ptr++, len--) {
		*(unsigned char *)ptr = (unsigned char)value;
	}

	atomic_thread_fence();

	return retlen;
}
