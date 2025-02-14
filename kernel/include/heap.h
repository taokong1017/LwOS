#ifndef __HEAP_H__
#define __HEAP_H__

#include <types.h>

#define CHUNK_UNIT 8U

enum chunk_fields { LEFT_SIZE, SIZE_AND_USED, FREE_PREV, FREE_NEXT };
typedef uint32_t chunkid_t;
typedef uint32_t chunksz_t;
typedef struct {
	char bytes[CHUNK_UNIT];
} chunk_unit_t;

struct heap_bucket {
	chunkid_t next;
};

struct heap {
	chunkid_t chunk0_hdr[2];
	chunkid_t end_chunk;
	uint32_t avail_buckets;
	size_t free_bytes;
	size_t allocated_bytes;
	size_t max_allocated_bytes;
	struct heap_bucket buckets[0];
};

static inline bool big_heap_chunks(chunksz_t chunks) {
	return chunks > 0x7fffU;
}

static inline bool big_heap_bytes(size_t bytes) {
	return big_heap_chunks(bytes / CHUNK_UNIT);
}

static inline bool big_heap(struct heap *h) {
	return big_heap_chunks(h->end_chunk);
}

static inline chunk_unit_t *chunk_buf(struct heap *h) {
	return (chunk_unit_t *)h;
}

static inline chunkid_t chunk_field(struct heap *h, chunkid_t c,
									enum chunk_fields f) {
	chunk_unit_t *buf = chunk_buf(h);
	void *cmem = &buf[c];

	if (big_heap(h)) {
		return ((uint32_t *)cmem)[f];
	} else {
		return ((uint16_t *)cmem)[f];
	}
}

static inline void chunk_set(struct heap *h, chunkid_t c, enum chunk_fields f,
							 chunkid_t val) {
	chunk_unit_t *buf = chunk_buf(h);
	void *cmem = &buf[c];

	if (big_heap(h)) {
		((uint32_t *)cmem)[f] = val;
	} else {
		((uint16_t *)cmem)[f] = val;
	}
}

static inline bool chunk_used(struct heap *h, chunkid_t c) {
	return chunk_field(h, c, SIZE_AND_USED) & 1U;
}

static inline chunksz_t chunk_size(struct heap *h, chunkid_t c) {
	return chunk_field(h, c, SIZE_AND_USED) >> 1;
}

static inline void set_chunk_used(struct heap *h, chunkid_t c, bool used) {
	chunk_unit_t *buf = chunk_buf(h);
	void *cmem = &buf[c];

	if (big_heap(h)) {
		if (used) {
			((uint32_t *)cmem)[SIZE_AND_USED] |= 1U;
		} else {
			((uint32_t *)cmem)[SIZE_AND_USED] &= ~1U;
		}
	} else {
		if (used) {
			((uint16_t *)cmem)[SIZE_AND_USED] |= 1U;
		} else {
			((uint16_t *)cmem)[SIZE_AND_USED] &= ~1U;
		}
	}
}

static inline void set_chunk_size(struct heap *h, chunkid_t c, chunksz_t size) {
	chunk_set(h, c, SIZE_AND_USED, size << 1);
}

static inline chunkid_t prev_free_chunk(struct heap *h, chunkid_t c) {
	return chunk_field(h, c, FREE_PREV);
}

static inline chunkid_t next_free_chunk(struct heap *h, chunkid_t c) {
	return chunk_field(h, c, FREE_NEXT);
}

static inline void set_prev_free_chunk(struct heap *h, chunkid_t c,
									   chunkid_t prev) {
	chunk_set(h, c, FREE_PREV, prev);
}

static inline void set_next_free_chunk(struct heap *h, chunkid_t c,
									   chunkid_t next) {
	chunk_set(h, c, FREE_NEXT, next);
}

static inline chunkid_t left_chunk(struct heap *h, chunkid_t c) {
	return c - chunk_field(h, c, LEFT_SIZE);
}

static inline chunkid_t right_chunk(struct heap *h, chunkid_t c) {
	return c + chunk_size(h, c);
}

static inline void set_left_chunk_size(struct heap *h, chunkid_t c,
									   chunksz_t size) {
	chunk_set(h, c, LEFT_SIZE, size);
}

static inline bool solo_free_header(struct heap *h, chunkid_t c) {
	return big_heap(h) && (chunk_size(h, c) == 1U);
}

static inline size_t chunk_header_bytes(struct heap *h) {
	return big_heap(h) ? 8 : 4;
}

static inline size_t heap_footer_bytes(size_t size) {
	return big_heap_bytes(size) ? 8 : 4;
}

static inline chunksz_t chunksz(size_t bytes) {
	return (bytes + CHUNK_UNIT - 1U) / CHUNK_UNIT;
}

static inline chunksz_t bytes_to_chunksz(struct heap *h, size_t bytes) {
	return chunksz(chunk_header_bytes(h) + bytes);
}

static inline chunksz_t min_chunk_size(struct heap *h) {
	return bytes_to_chunksz(h, 1);
}

static inline size_t chunksz_to_bytes(struct heap *h, chunksz_t chunksz_in) {
	return chunksz_in * CHUNK_UNIT - chunk_header_bytes(h);
}

static inline int bucket_idx(struct heap *h, chunksz_t sz) {
	unsigned int usable_sz = sz - min_chunk_size(h) + 1;
	return 31 - __builtin_clz(usable_sz);
}

static inline bool size_too_big(struct heap *h, size_t bytes) {
	return (bytes / CHUNK_UNIT) >= h->end_chunk;
}

static inline void get_alloc_info(struct heap *h, size_t *alloc_bytes,
								  size_t *free_bytes) {
	chunkid_t c;

	*alloc_bytes = 0;
	*free_bytes = 0;

	for (c = right_chunk(h, 0); c < h->end_chunk; c = right_chunk(h, c)) {
		if (chunk_used(h, c)) {
			*alloc_bytes += chunksz_to_bytes(h, chunk_size(h, c));
		} else if (!solo_free_header(h, c)) {
			*free_bytes += chunksz_to_bytes(h, chunk_size(h, c));
		}
	}
}

#endif
