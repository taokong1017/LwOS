#include <user_heap.h>
#include <general.h>
#include <string.h>
#include <compiler.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ROUND_UP(x, align)                                                     \
	((((unsigned long)(x) + ((unsigned long)(align)-1)) /                      \
	  (unsigned long)(align)) *                                                \
	 (unsigned long)(align))
#define ROUND_DOWN(x, align)                                                   \
	(((unsigned long)(x) / (unsigned long)(align)) * (unsigned long)(align))

static inline void increase_allocated_bytes(struct heap *h, size_t num_bytes) {
	h->allocated_bytes += num_bytes;
	h->max_allocated_bytes = MAX(h->max_allocated_bytes, h->allocated_bytes);
}

static void *chunk_mem(struct heap *h, chunkid_t c) {
	chunk_unit_t *buf = chunk_buf(h);
	uint8_t *ret = ((uint8_t *)&buf[c]) + chunk_header_bytes(h);

	return ret;
}

static void free_list_remove_bidx(struct heap *h, chunkid_t c, int bidx) {
	struct heap_bucket *b = &h->buckets[bidx];

	if (next_free_chunk(h, c) == c) {
		h->avail_buckets &= ~BIT(bidx);
		b->next = 0;
	} else {
		chunkid_t first = prev_free_chunk(h, c), second = next_free_chunk(h, c);

		b->next = second;
		set_next_free_chunk(h, first, second);
		set_prev_free_chunk(h, second, first);
	}

	h->free_bytes -= chunksz_to_bytes(h, chunk_size(h, c));
}

static void free_list_remove(struct heap *h, chunkid_t c) {
	if (!solo_free_header(h, c)) {
		int bidx = bucket_idx(h, chunk_size(h, c));
		free_list_remove_bidx(h, c, bidx);
	}
}

static void free_list_add_bidx(struct heap *h, chunkid_t c, int bidx) {
	struct heap_bucket *b = &h->buckets[bidx];

	if (b->next == 0U) {
		/* Empty list, first item */
		h->avail_buckets |= BIT(bidx);
		b->next = c;
		set_prev_free_chunk(h, c, c);
		set_next_free_chunk(h, c, c);
	} else {
		/* Insert before (!) the "next" pointer */
		chunkid_t second = b->next;
		chunkid_t first = prev_free_chunk(h, second);

		set_prev_free_chunk(h, c, first);
		set_next_free_chunk(h, c, second);
		set_next_free_chunk(h, first, c);
		set_prev_free_chunk(h, second, c);
	}

	h->free_bytes += chunksz_to_bytes(h, chunk_size(h, c));
}

static void free_list_add(struct heap *h, chunkid_t c) {
	if (!solo_free_header(h, c)) {
		int bidx = bucket_idx(h, chunk_size(h, c));
		free_list_add_bidx(h, c, bidx);
	}
}

/* Splits a chunk "lc" into a left chunk and a right chunk at "rc".
 * Leaves both chunks marked "free"
 */
static void split_chunks(struct heap *h, chunkid_t lc, chunkid_t rc) {
	chunksz_t sz0 = chunk_size(h, lc);
	chunksz_t lsz = rc - lc;
	chunksz_t rsz = sz0 - lsz;

	set_chunk_size(h, lc, lsz);
	set_chunk_size(h, rc, rsz);
	set_left_chunk_size(h, rc, lsz);
	set_left_chunk_size(h, right_chunk(h, rc), rsz);
}

static void merge_chunks(struct heap *h, chunkid_t lc, chunkid_t rc) {
	chunksz_t newsz = chunk_size(h, lc) + chunk_size(h, rc);

	set_chunk_size(h, lc, newsz);
	set_left_chunk_size(h, right_chunk(h, rc), newsz);
}

static void free_chunk(struct heap *h, chunkid_t c) {
	/* Merge with free right chunk? */
	if (!chunk_used(h, right_chunk(h, c))) {
		free_list_remove(h, right_chunk(h, c));
		merge_chunks(h, c, right_chunk(h, c));
	}

	/* Merge with free left chunk? */
	if (!chunk_used(h, left_chunk(h, c))) {
		free_list_remove(h, left_chunk(h, c));
		merge_chunks(h, left_chunk(h, c), c);
		c = left_chunk(h, c);
	}

	free_list_add(h, c);
}

/*
 * Return the closest chunk ID corresponding to given memory pointer.
 * Here "closest" is only meaningful in the context of user_heap_aligned_alloc()
 * where wanted alignment might not always correspond to a chunk header
 * boundary.
 */
static chunkid_t mem_to_chunkid(struct heap *h, void *p) {
	uint8_t *mem = p, *base = (uint8_t *)chunk_buf(h);
	return (mem - chunk_header_bytes(h) - base) / CHUNK_UNIT;
}

void user_heap_free(struct user_heap *heap, void *mem) {
	if (mem == NULL) {
		return; /* ISO C free() semantics */
	}

	struct heap *h = heap->heap;
	chunkid_t c = mem_to_chunkid(h, mem);

	/*
	 * This should catch many double-free cases.
	 * This is cheap enough so let's do it all the time.
	 */
	assert(chunk_used(h, c),
		   "unexpected heap state (double-free?) for memory at %p", mem);

	/*
	 * It is easy to catch many common memory overflow cases with
	 * a quick check on this and next chunk header fields that are
	 * immediately before and after the freed memory.
	 */
	assert(left_chunk(h, right_chunk(h, c)) == c,
		   "corrupted heap bounds (buffer overflow?) for memory at %p", mem);

	set_chunk_used(h, c, false);
	h->allocated_bytes -= chunksz_to_bytes(h, chunk_size(h, c));

	free_chunk(h, c);
}

static chunkid_t alloc_chunk(struct heap *h, chunksz_t sz) {
	int bi = bucket_idx(h, sz);
	struct heap_bucket *b = &h->buckets[bi];

	/* First try a bounded count of items from the minimal bucket
	 * size.  These may not fit, trying (e.g.) three means that
	 * (assuming that chunk sizes are evenly distributed[1]) we
	 * have a 7/8 chance of finding a match, thus keeping the
	 * number of such blocks consumed by allocation higher than
	 * the number of smaller blocks created by fragmenting larger
	 * ones.
	 *
	 * [1] In practice, they are never evenly distributed, of
	 * course.  But even in pathological situations we still
	 * maintain our constant time performance and at worst see
	 * fragmentation waste of the order of the block allocated
	 * only.
	 */
	if (b->next != 0U) {
		chunkid_t first = b->next;
		int i = 3;
		do {
			chunkid_t c = b->next;
			if (chunk_size(h, c) >= sz) {
				free_list_remove_bidx(h, c, bi);
				return c;
			}
			b->next = next_free_chunk(h, c);
		} while (--i && b->next != first);
	}

	/* Otherwise pick the smallest non-empty bucket guaranteed to
	 * fit and use that unconditionally.
	 */
	uint32_t bmask = h->avail_buckets & ~(BIT_MASK(bi + 1));

	if (bmask != 0U) {
		int minbucket = __builtin_ctz(bmask);
		chunkid_t c = h->buckets[minbucket].next;

		free_list_remove_bidx(h, c, minbucket);
		return c;
	}

	return 0;
}

void *user_heap_alloc(struct user_heap *heap, size_t bytes) {
	struct heap *h = heap->heap;
	void *mem;

	if ((bytes == 0U) || size_too_big(h, bytes)) {
		return NULL;
	}

	chunksz_t chunk_sz = bytes_to_chunksz(h, bytes);
	chunkid_t c = alloc_chunk(h, chunk_sz);
	if (c == 0U) {
		return NULL;
	}

	/* Split off remainder if any */
	if (chunk_size(h, c) > chunk_sz) {
		split_chunks(h, c, c + chunk_sz);
		free_list_add(h, c + chunk_sz);
	}

	set_chunk_used(h, c, true);
	mem = chunk_mem(h, c);
	increase_allocated_bytes(h, chunksz_to_bytes(h, chunk_size(h, c)));

	return mem;
}

void *user_heap_aligned_alloc(struct user_heap *heap, size_t align,
							  size_t bytes) {
	struct heap *h = heap->heap;
	size_t gap, rew;

	/*
	 * Split align and rewind values (if any).
	 * We allow for one bit of rewind in addition to the alignment
	 * value to efficiently accommodate heap_aligned_alloc().
	 * So if e.g. align = 0x28 (32 | 8) this means we align to a 32-byte
	 * boundary and then rewind 8 bytes.
	 */
	rew = align & -align;
	if (align != rew) {
		align -= rew;
		gap = MIN(rew, chunk_header_bytes(h));
	} else {
		if (align <= chunk_header_bytes(h)) {
			return user_heap_alloc(heap, bytes);
		}
		rew = 0;
		gap = chunk_header_bytes(h);
	}

	assert((align & (align - 1)) == 0, "align must be a power of 2");

	if ((bytes == 0) || size_too_big(h, bytes)) {
		return NULL;
	}

	/*
	 * Find a free block that is guaranteed to fit.
	 * We over-allocate to account for alignment and then free
	 * the extra allocations afterwards.
	 */
	chunksz_t padded_sz = bytes_to_chunksz(h, bytes + align - gap);
	chunkid_t c0 = alloc_chunk(h, padded_sz);

	if (c0 == 0) {
		return NULL;
	}
	uint8_t *mem = chunk_mem(h, c0);

	/* Align allocated memory */
	mem = (uint8_t *)ROUND_UP(mem + rew, align) - rew;
	chunk_unit_t *end = (chunk_unit_t *)ROUND_UP(mem + bytes, CHUNK_UNIT);

	/* Get corresponding chunks */
	chunkid_t c = mem_to_chunkid(h, mem);
	chunkid_t c_end = end - chunk_buf(h);

	/* Split and free unused prefix */
	if (c > c0) {
		split_chunks(h, c0, c);
		free_list_add(h, c0);
	}

	/* Split and free unused suffix */
	if (right_chunk(h, c) > c_end) {
		split_chunks(h, c, c_end);
		free_list_add(h, c_end);
	}

	set_chunk_used(h, c, true);

	increase_allocated_bytes(h, chunksz_to_bytes(h, chunk_size(h, c)));

	return mem;
}

void *user_heap_aligned_realloc(struct user_heap *heap, void *ptr, size_t align,
								size_t bytes) {
	struct heap *h = heap->heap;

	/* special realloc semantics */
	if (ptr == NULL) {
		return user_heap_aligned_alloc(heap, align, bytes);
	}
	if (bytes == 0) {
		user_heap_free(heap, ptr);
		return NULL;
	}

	assert((align & (align - 1)) == 0, "align must be a power of 2");

	if (size_too_big(h, bytes)) {
		return NULL;
	}

	chunkid_t c = mem_to_chunkid(h, ptr);
	chunkid_t rc = right_chunk(h, c);
	size_t align_gap = (uint8_t *)ptr - (uint8_t *)chunk_mem(h, c);
	chunksz_t chunks_need = bytes_to_chunksz(h, bytes + align_gap);

	if (align && ((uintptr_t)ptr & (align - 1))) {
		/* ptr is not sufficiently aligned */
	} else if (chunk_size(h, c) == chunks_need) {
		/* We're good already */
		return ptr;
	} else if (chunk_size(h, c) > chunks_need) {
		/* Shrink in place, split off and free unused suffix */
		h->allocated_bytes -= (chunk_size(h, c) - chunks_need) * CHUNK_UNIT;
		split_chunks(h, c, c + chunks_need);
		set_chunk_used(h, c, true);
		free_chunk(h, c + chunks_need);

		return ptr;
	} else if (!chunk_used(h, rc) &&
			   (chunk_size(h, c) + chunk_size(h, rc) >= chunks_need)) {
		/* Expand: split the right chunk and append */
		chunksz_t split_size = chunks_need - chunk_size(h, c);

		increase_allocated_bytes(h, split_size * CHUNK_UNIT);
		free_list_remove(h, rc);

		if (split_size < chunk_size(h, rc)) {
			split_chunks(h, rc, rc + split_size);
			free_list_add(h, rc + split_size);
		}

		merge_chunks(h, c, rc);
		set_chunk_used(h, c, true);
		return ptr;
	}

	/*
	 * Fallback: allocate and copy
	 *
	 * Note for heap listener notification:
	 * The calls to allocation and free functions generate
	 * notification already, so there is no need to those here.
	 */
	void *ptr2 = user_heap_aligned_alloc(heap, align, bytes);

	if (ptr2 != NULL) {
		size_t prev_size = chunksz_to_bytes(h, chunk_size(h, c)) - align_gap;

		memcpy(ptr2, ptr, MIN(prev_size, bytes));
		user_heap_free(heap, ptr);
	}
	return ptr2;
}

void user_heap_init(struct user_heap *heap, void *mem, size_t bytes) {
	assert((bytes / CHUNK_UNIT) <= 0x7fffffffU, "user heap size is too big");
	assert(bytes > heap_footer_bytes(bytes), "user heap size is too small");

	bytes -= heap_footer_bytes(bytes);

	/* Round the start up, the end down */
	uintptr_t addr = ROUND_UP(mem, CHUNK_UNIT);
	uintptr_t end = ROUND_DOWN((uint8_t *)mem + bytes, CHUNK_UNIT);
	chunksz_t heap_sz = (end - addr) / CHUNK_UNIT;

	assert(heap_sz > chunksz(sizeof(struct heap)), "heap size is too small");

	struct heap *h = (struct heap *)addr;
	heap->heap = h;
	h->end_chunk = heap_sz;
	h->avail_buckets = 0;
	h->free_bytes = 0;
	h->allocated_bytes = 0;
	h->max_allocated_bytes = 0;

	int nb_buckets = bucket_idx(h, heap_sz) + 1;
	chunksz_t chunk0_size =
		chunksz(sizeof(struct heap) + nb_buckets * sizeof(struct heap_bucket));

	assert((chunk0_size + min_chunk_size(h)) <= heap_sz,
		   "user heap size is too small");

	for (int i = 0; i < nb_buckets; i++) {
		h->buckets[i].next = 0;
	}

	/* chunk containing our struct z_heap */
	set_chunk_size(h, 0, chunk0_size);
	set_left_chunk_size(h, 0, 0);
	set_chunk_used(h, 0, true);

	/* chunk containing the free heap */
	set_chunk_size(h, chunk0_size, heap_sz - chunk0_size);
	set_left_chunk_size(h, chunk0_size, chunk0_size);

	/* the end marker chunk */
	set_chunk_size(h, heap_sz, 0);
	set_left_chunk_size(h, heap_sz, heap_sz - chunk0_size);
	set_chunk_used(h, heap_sz, true);

	free_list_add(h, chunk0_size);
}
