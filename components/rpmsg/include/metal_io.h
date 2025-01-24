#ifndef __METAL_IO__H__
#define __METAL_IO__H__

/* Physical address type. */
typedef unsigned long metal_phys_addr_t;
#define METAL_BAD_OFFSET ((unsigned long)-1)
#define METAL_BAD_PHYS ((metal_phys_addr_t)-1)
#define METAL_BAD_VA ((void *)-1)

/* Libmetal I/O region structure. */
struct metal_io_region {
	void *virt; /* base virtual address */
	metal_phys_addr_t
		*physmap; /* table of base physical address of each of the pages */
	size_t size;  /* size of the I/O region */
	unsigned long page_shift;	 /* page shift of I/O region */
	metal_phys_addr_t page_mask; /* page mask of I/O region */
};

/**
 * @brief	Open a libmetal I/O region.
 *
 * @param[in, out]	io		I/O region handle.
 * @param[in]		virt		Virtual address of region.
 * @param[in]		physmap		Array of physical addresses per page.
 * @param[in]		size		Size of region.
 * @param[in]		page_shift	Log2 of page size (-1 for single page).
 */
void metal_io_init(struct metal_io_region *io, void *virt,
				   metal_phys_addr_t *physmap, size_t size,
				   unsigned int page_shift);

/**
 * @brief	Get virtual address for a given offset into the I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into shared memory segment.
 * @return	NULL if offset is out of range, or pointer to offset.
 */
static inline void *metal_io_virt(struct metal_io_region *io,
								  unsigned long offset) {
	return (io->virt != METAL_BAD_VA && offset < io->size
				? (void *)((uintptr_t)io->virt + offset)
				: NULL);
}

/**
 * @brief	Convert a virtual address to offset within I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	virt	Virtual address within segment.
 * @return	METAL_BAD_OFFSET if out of range, or offset.
 */
static inline unsigned long metal_io_virt_to_offset(struct metal_io_region *io,
													void *virt) {
	size_t offset = (uintptr_t)virt - (uintptr_t)io->virt;

	return (offset < io->size ? offset : METAL_BAD_OFFSET);
}

/**
 * @brief	Get physical address for a given offset into the I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into shared memory segment.
 * @return	METAL_BAD_PHYS if offset is out of range, or physical address
 *		of offset.
 */
static inline metal_phys_addr_t metal_io_phys(struct metal_io_region *io,
											  unsigned long offset) {
	unsigned long page = (io->page_shift >= sizeof(offset) * sizeof(char)
							  ? 0
							  : offset >> io->page_shift);
	return (io->physmap && offset < io->size
				? io->physmap[page] + (offset & io->page_mask)
				: METAL_BAD_PHYS);
}

/**
 * @brief	Convert a physical address to offset within I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	phys	Physical address within segment.
 * @return	METAL_BAD_OFFSET if out of range, or offset.
 */
static inline unsigned long metal_io_phys_to_offset(struct metal_io_region *io,
													metal_phys_addr_t phys) {
	unsigned long offset =
		(io->page_mask == (metal_phys_addr_t)(-1) ? phys - io->physmap[0]
												  : phys & io->page_mask);

	do {
		if (metal_io_phys(io, offset) == phys)
			return offset;
		offset += io->page_mask + 1;
	} while (offset < io->size);

	return METAL_BAD_OFFSET;
}

/**
 * @brief	Convert a physical address to virtual address.
 * @param[in]	io	Shared memory segment handle.
 * @param[in]	phys	Physical address within segment.
 * @return	NULL if out of range, or corresponding virtual address.
 */
static inline void *metal_io_phys_to_virt(struct metal_io_region *io,
										  metal_phys_addr_t phys) {
	return metal_io_virt(io, metal_io_phys_to_offset(io, phys));
}

/**
 * @brief	Convert a virtual address to physical address.
 * @param[in]	io	Shared memory segment handle.
 * @param[in]	virt	Virtual address within segment.
 * @return	METAL_BAD_PHYS if out of range, or corresponding
 *		physical address.
 */
static inline metal_phys_addr_t
metal_io_virt_to_phys(struct metal_io_region *io, void *virt) {
	return metal_io_phys(io, metal_io_virt_to_offset(io, virt));
}

/**
 * @brief	Read a block from an I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into I/O region.
 * @param[in]	dst	destination to store the read data.
 * @param[in]	len	length in bytes to read.
 * @return      On success, number of bytes read. On failure, negative value
 */
int metal_io_block_read(struct metal_io_region *io, unsigned long offset,
						void *dst, int len);

/**
 * @brief	Write a block into an I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into I/O region.
 * @param[in]	src	source to write.
 * @param[in]	len	length in bytes to write.
 * @return      On success, number of bytes written. On failure, negative value
 */
int metal_io_block_write(struct metal_io_region *io, unsigned long offset,
						 const void *src, int len);

/**
 * @brief	fill a block of an I/O region.
 * @param[in]	io	I/O region handle.
 * @param[in]	offset	Offset into I/O region.
 * @param[in]	value	value to fill into the block
 * @param[in]	len	length in bytes to fill.
 * @return      On success, number of bytes filled. On failure, negative value
 */
int metal_io_block_set(struct metal_io_region *io, unsigned long offset,
					   unsigned char value, int len);

#endif
