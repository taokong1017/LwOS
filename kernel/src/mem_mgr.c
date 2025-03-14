#include <mem_mgr.h>
#include <mutex.h>
#include <log.h>
#include <string.h>
#include <task_sched.h>
#include <mutex.h>
#include <task.h>
#include <limits.h>

#define MEM_TAG "MEM"
#define MEM_MUTEX_NAME "kmem_mutex"
#define MAX_FL_INDEX (30)
#define FL_INDEX_OFFSET (6)
#define FL_INDEX_NUM (MAX_FL_INDEX - FL_INDEX_OFFSET)

#define MAX_SL_INDEX_LOG2 (5)
#define MAX_SL_INDEX_NUM (1 << MAX_SL_INDEX_LOG2)

#define SMALL_BLOCK (128)
#define MIN_BLOCK_SIZE (sizeof(struct free_ptr))
#define BLOCK_ALIGN (sizeof(void *) * 2)

#define BLOCK_STATE (0x1)
#define PREV_STATE (0x2)
#define FREE_BLOCK (0x1)
#define USED_BLOCK (0x0)
#define PREV_FREE (0x2)
#define PREV_USED (0x0)
#define SIGNATURE (0x2A59FA59)

#define PTR_MASK (BLOCK_ALIGN - 1)
#define BLOCK_SIZE (0xFFFFFFFF - PTR_MASK)
#define BHDR_OVERHEAD (sizeof(struct bhdr) - MIN_BLOCK_SIZE)

#define GET_NEXT_BLOCK(_addr, _r) ((struct bhdr *)((char *)(_addr) + (_r)))
#define MEM_ALIGN ((BLOCK_ALIGN)-1)
#define ROUNDUP_SIZE(_r) (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
#define ROUNDDOWN_SIZE(_r) ((_r) & ~MEM_ALIGN)
#define ROUNDUP(_x, _v) ((((~(_x)) + 1) & ((_v)-1)) + (_x))

extern char __kernel_heap_start[];
extern char __kernel_heap_end[];

struct free_ptr {
	struct bhdr *prev;
	struct bhdr *next;
};

struct bhdr {
	struct bhdr *prev_hdr;
	uint32_t size;

	union {
		struct free_ptr free_ptr;
		uint8_t buffer[1];
	} ptr;
};

struct area_info {
	struct bhdr *end;
	struct area_info *next;
};

struct tlsf {
	uint32_t signature;

	uint64_t used_size;
	uint64_t max_size;

	struct area_info *area_head;
	uint32_t fl_bitmap;

	uint32_t sl_bitmap[FL_INDEX_NUM];
	struct bhdr *matrix[FL_INDEX_NUM][MAX_SL_INDEX_NUM];
};

static const int32_t table[] = {
	-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
	4,	4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5,	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,
	6,	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6,	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6,	6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7,	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

static uint8_t *mp = NULL;
static struct mutex mem_mutex;

static int32_t ls_bit(int32_t i) {
	uint32_t a = 0;
	uint32_t x = i & -i;

	a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
	return table[x >> a] + a;
}

static int32_t ms_bit(int32_t i) {
	uint32_t a = 0;
	uint32_t x = (uint32_t)i;

	a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
	return table[x >> a] + a;
}

static void set_bit(int32_t nr, uint32_t *addr) {
	addr[nr >> 5] |= 1 << (nr & 0x1f);
}

static void clear_bit(int32_t nr, uint32_t *addr) {
	addr[nr >> 5] &= ~(1 << (nr & 0x1f));
}

static void mapping_serach(uint32_t *_r, uint32_t *_fl, uint32_t *_sl) {
	int32_t _t;

	if (*_r < SMALL_BLOCK) {
		*_fl = 0;
		*_sl = *_r / (SMALL_BLOCK / MAX_SL_INDEX_NUM);
	} else {
		_t = (1 << (ms_bit(*_r) - MAX_SL_INDEX_LOG2)) - 1;
		*_r = *_r + _t;
		*_fl = ms_bit(*_r);
		*_sl = (*_r >> (*_fl - MAX_SL_INDEX_LOG2)) - MAX_SL_INDEX_NUM;
		*_fl -= FL_INDEX_OFFSET;
		*_r &= ~_t;
	}
}

static void mapping_insert(uint32_t _r, uint32_t *_fl, uint32_t *_sl) {
	if (_r < SMALL_BLOCK) {
		*_fl = 0;
		*_sl = _r / (SMALL_BLOCK / MAX_SL_INDEX_NUM);
	} else {
		*_fl = ms_bit(_r);
		*_sl = (_r >> (*_fl - MAX_SL_INDEX_LOG2)) - MAX_SL_INDEX_NUM;
		*_fl -= FL_INDEX_OFFSET;
	}
}

static struct bhdr *find_suitable_block(struct tlsf *_tlsf, uint32_t *_fl,
										uint32_t *_sl) {
	uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (U32_MAX << *_sl);
	struct bhdr *_b = NULL;

	if (_tmp) {
		*_sl = ls_bit(_tmp);
		_b = _tlsf->matrix[*_fl][*_sl];
	} else {
		*_fl = ls_bit(_tlsf->fl_bitmap & (U32_MAX << (*_fl + 1)));
		if (*_fl > 0) {
			*_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
			_b = _tlsf->matrix[*_fl][*_sl];
		}
	}
	return _b;
}

#define extract_block_hdr(_b, _tlsf, _fl, _sl)                                 \
	do {                                                                       \
		_tlsf->matrix[_fl][_sl] = _b->ptr.free_ptr.next;                       \
		if (_tlsf->matrix[_fl][_sl])                                           \
			_tlsf->matrix[_fl][_sl]->ptr.free_ptr.prev = NULL;                 \
		else {                                                                 \
			clear_bit(_sl, &_tlsf->sl_bitmap[_fl]);                            \
			if (!_tlsf->sl_bitmap[_fl])                                        \
				clear_bit(_fl, &_tlsf->fl_bitmap);                             \
		}                                                                      \
		_b->ptr.free_ptr.prev = NULL;                                          \
		_b->ptr.free_ptr.next = NULL;                                          \
	} while (0)

#define extract_block(_b, _tlsf, _fl, _sl)                                     \
	do {                                                                       \
		if (_b->ptr.free_ptr.next)                                             \
			_b->ptr.free_ptr.next->ptr.free_ptr.prev = _b->ptr.free_ptr.prev;  \
		if (_b->ptr.free_ptr.prev)                                             \
			_b->ptr.free_ptr.prev->ptr.free_ptr.next = _b->ptr.free_ptr.next;  \
		if (_tlsf->matrix[_fl][_sl] == _b) {                                   \
			_tlsf->matrix[_fl][_sl] = _b->ptr.free_ptr.next;                   \
			if (!_tlsf->matrix[_fl][_sl]) {                                    \
				clear_bit(_sl, &_tlsf->sl_bitmap[_fl]);                        \
				if (!_tlsf->sl_bitmap[_fl])                                    \
					clear_bit(_fl, &_tlsf->fl_bitmap);                         \
			}                                                                  \
		}                                                                      \
		_b->ptr.free_ptr.prev = NULL;                                          \
		_b->ptr.free_ptr.next = NULL;                                          \
	} while (0)

#define insert_block(_b, _tlsf, _fl, _sl)                                      \
	do {                                                                       \
		_b->ptr.free_ptr.prev = NULL;                                          \
		_b->ptr.free_ptr.next = _tlsf->matrix[_fl][_sl];                       \
		if (_tlsf->matrix[_fl][_sl]) {                                         \
			_tlsf->matrix[_fl][_sl]->ptr.free_ptr.prev = _b;                   \
		}                                                                      \
		_tlsf->matrix[_fl][_sl] = _b;                                          \
		set_bit(_sl, &_tlsf->sl_bitmap[_fl]);                                  \
		set_bit(_fl, &_tlsf->fl_bitmap);                                       \
	} while (0)

#define tlsf_add_size(tlsf, b)                                                 \
	do {                                                                       \
		tlsf->used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;             \
		if (tlsf->used_size > tlsf->max_size) {                                \
			tlsf->max_size = tlsf->used_size;                                  \
		}                                                                      \
	} while (0)

#define tlsf_remove_size(tlsf, b)                                              \
	do {                                                                       \
		tlsf->used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;             \
	} while (0)

static struct bhdr *process_area(void *area, uint32_t size) {
	struct bhdr *b, *lb, *ib;
	struct area_info *ai;

	ib = (struct bhdr *)area;
	ib->size =
		(sizeof(struct area_info) < MIN_BLOCK_SIZE)
			? MIN_BLOCK_SIZE
			: ROUNDUP_SIZE(sizeof(struct area_info)) | USED_BLOCK | PREV_USED;
	b = (struct bhdr *)GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
	b->size =
		ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) |
		USED_BLOCK | PREV_USED;
	b->ptr.free_ptr.prev = b->ptr.free_ptr.next = 0;
	lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
	lb->prev_hdr = b;
	lb->size = 0 | USED_BLOCK | PREV_FREE;
	ai = (struct area_info *)ib->ptr.buffer;
	ai->next = 0;
	ai->end = lb;

	return ib;
}

static void free_ex(void *ptr, void *mem_pool) {
	struct tlsf *tlsf = (struct tlsf *)mem_pool;
	struct bhdr *b = NULL, *tmp_b = NULL;
	uint32_t fl = 0, sl = 0;

	if (!ptr || !mem_pool) {
		return;
	}

	b = (struct bhdr *)((char *)ptr - BHDR_OVERHEAD);
	b->size |= FREE_BLOCK;

	tlsf_remove_size(tlsf, b);

	b->ptr.free_ptr.prev = NULL;
	b->ptr.free_ptr.next = NULL;
	tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
	if (tmp_b->size & FREE_BLOCK) {
		mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
		extract_block(tmp_b, tlsf, fl, sl);
		b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
	}
	if (b->size & PREV_FREE) {
		tmp_b = b->prev_hdr;
		mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
		extract_block(tmp_b, tlsf, fl, sl);
		tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
		b = tmp_b;
	}
	mapping_insert(b->size & BLOCK_SIZE, &fl, &sl);
	insert_block(b, tlsf, fl, sl);

	tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
	tmp_b->size |= PREV_FREE;
	tmp_b->prev_hdr = b;
}

static errno_t kmem_init(void *mem, uint32_t size) {
	struct tlsf *tlsf;
	struct bhdr *b, *ib;

	if (!mem) {
		log_err(MEM_TAG, "memory pool is NULL\n");
		return ERRNO_MEM_NULL_PTR;
	}

	if (!size || size < sizeof(struct tlsf) + BHDR_OVERHEAD * 8) {
		log_err(MEM_TAG, "memory pool size is not enough\n");
		return ERRNO_MEM_NO_MEMORY;
	}

	if (((uintptr_t)mem & PTR_MASK)) {
		log_err(MEM_TAG, "memory pool address is not aligned\n");
		return ERRNO_MEM_NO_ALIGN;
	}

	tlsf = (struct tlsf *)mem;
	if (tlsf->signature == SIGNATURE) {
		mp = mem;
		b = GET_NEXT_BLOCK(mp, ROUNDUP_SIZE(sizeof(struct tlsf)));
		return b->size & BLOCK_SIZE;
	}

	mp = mem;
	memset(mem, 0, sizeof(struct tlsf));
	tlsf->signature = SIGNATURE;

	ib = process_area(GET_NEXT_BLOCK(mem, ROUNDUP_SIZE(sizeof(struct tlsf))),
					  ROUNDDOWN_SIZE(size - sizeof(struct tlsf)));
	b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
	free_ex(b->ptr.buffer, tlsf);
	tlsf->area_head = (struct area_info *)ib->ptr.buffer;

	tlsf->used_size = size - (b->size & BLOCK_SIZE);
	tlsf->max_size = tlsf->used_size;

	return OK;
}

void kheap_init() {
	mutex_init(&mem_mutex, MEM_MUTEX_NAME);
	kmem_init((void *)__kernel_heap_start,
			  (uintptr_t)__kernel_heap_end - (uintptr_t)__kernel_heap_start);
}

static void *malloc_ex(uint32_t size, void *mem_pool) {
	struct tlsf *tlsf = (struct tlsf *)mem_pool;
	struct bhdr *b = NULL, *b2 = NULL, *next_b = NULL;
	uint32_t fl = 0, sl = 0;
	uint32_t tmp_size;

	size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(size);

	mapping_serach(&size, &fl, &sl);
	b = find_suitable_block(tlsf, &fl, &sl);
	if (!b) {
		return NULL;
	}

	extract_block_hdr(b, tlsf, fl, sl);
	next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);

	tmp_size = (b->size & BLOCK_SIZE) - size;
	if (tmp_size >= sizeof(struct bhdr)) {
		tmp_size -= BHDR_OVERHEAD;
		b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
		b2->size = tmp_size | FREE_BLOCK | PREV_USED;
		next_b->prev_hdr = b2;
		mapping_insert(tmp_size, &fl, &sl);
		insert_block(b2, tlsf, fl, sl);
		b->size = size | (b->size & PREV_STATE);
	} else {
		next_b->size &= (~PREV_FREE);
		b->size &= (~FREE_BLOCK);
	}

	tlsf_add_size(tlsf, b);

	return (void *)b->ptr.buffer;
}

void *kmalloc(uint32_t size) {
	void *ptr = NULL;
	struct task *task = current_task_get();

	if (!task) {
		return malloc_ex(size, mp);
	}

	mutex_take(mem_mutex.id, MUTEX_WAIT_FOREVER);
	ptr = malloc_ex(size, mp);
	mutex_give(mem_mutex.id);

	return ptr;
}

void kfree(void *ptr) {
	mutex_take(mem_mutex.id, MUTEX_WAIT_FOREVER);
	free_ex(ptr, mp);
	mutex_give(mem_mutex.id);
}

static void *realloc_ex(void *ptr, uint32_t new_size, void *mem_pool) {
	struct tlsf *tlsf = (struct tlsf *)mem_pool;
	void *ptr_aux = NULL;
	uint32_t cpsize = 0;
	struct bhdr *b = NULL, *tmp_b = NULL, *next_b = NULL;
	uint32_t fl = 0, sl = 0;
	uint32_t tmp_size;

	if (!ptr) {
		return malloc_ex(new_size, mem_pool);
	}

	if (!new_size) {
		free_ex(ptr, mem_pool);
		return NULL;
	}

	b = (struct bhdr *)((uint8_t *)ptr - BHDR_OVERHEAD);
	next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
	new_size =
		(new_size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : ROUNDUP_SIZE(new_size);
	tmp_size = (b->size & BLOCK_SIZE);
	if (new_size <= tmp_size) {
		tlsf_remove_size(tlsf, b);
		if (next_b->size & FREE_BLOCK) {
			mapping_insert(next_b->size & BLOCK_SIZE, &fl, &sl);
			extract_block(next_b, tlsf, fl, sl);
			tmp_size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
			next_b =
				GET_NEXT_BLOCK(next_b->ptr.buffer, next_b->size & BLOCK_SIZE);
		}
		tmp_size -= new_size;
		if (tmp_size >= sizeof(struct bhdr)) {
			tmp_size -= BHDR_OVERHEAD;
			tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, new_size);
			tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
			next_b->prev_hdr = tmp_b;
			next_b->size |= PREV_FREE;
			mapping_insert(tmp_size, &fl, &sl);
			insert_block(tmp_b, tlsf, fl, sl);
			b->size = new_size | (b->size & PREV_STATE);
		}
		tlsf_add_size(tlsf, b);
		return (void *)b->ptr.buffer;
	}
	if ((next_b->size & FREE_BLOCK)) {
		if (new_size <= (tmp_size + (next_b->size & BLOCK_SIZE))) {
			tlsf_remove_size(tlsf, b);
			mapping_insert(next_b->size & BLOCK_SIZE, &fl, &sl);
			extract_block(next_b, tlsf, fl, sl);
			b->size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
			next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
			next_b->prev_hdr = b;
			next_b->size &= ~PREV_FREE;
			tmp_size = (b->size & BLOCK_SIZE) - new_size;
			if (tmp_size >= sizeof(struct bhdr)) {
				tmp_size -= BHDR_OVERHEAD;
				tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, new_size);
				tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
				next_b->prev_hdr = tmp_b;
				next_b->size |= PREV_FREE;
				mapping_insert(tmp_size, &fl, &sl);
				insert_block(tmp_b, tlsf, fl, sl);
				b->size = new_size | (b->size & PREV_STATE);
			}
			tlsf_add_size(tlsf, b);
			return (void *)b->ptr.buffer;
		}
	}

	ptr_aux = malloc_ex(new_size, mem_pool);
	cpsize =
		((b->size & BLOCK_SIZE) > new_size) ? new_size : (b->size & BLOCK_SIZE);
	memcpy(ptr_aux, ptr, cpsize);
	free_ex(ptr, mem_pool);

	return ptr_aux;
}

void *krealloc(void *ptr, uint32_t size) {
	void *new_ptr = NULL;
	struct task *task = current_task_get();

	if (!task) {
		return realloc_ex(ptr, size, mp);
	}

	mutex_take(mem_mutex.id, MUTEX_WAIT_FOREVER);
	new_ptr = realloc_ex(ptr, size, mp);
	mutex_give(mem_mutex.id);

	return new_ptr;
}

static void *calloc_ex(size_t nelem, size_t elem_size, void *mem_pool) {
	void *ptr = NULL;

	if (nelem == 0 || elem_size == 0) {
		return NULL;
	}

	if (!(ptr = malloc_ex(nelem * elem_size, mem_pool))) {
		return NULL;
	}

	memset(ptr, 0, nelem * elem_size);

	return ptr;
}

void *kcalloc(uint32_t nelem, uint32_t elem_size) {
	void *ptr = NULL;
	struct task *task = current_task_get();

	if (!task) {
		return calloc_ex(nelem, elem_size, mp);
	}

	mutex_take(mem_mutex.id, MUTEX_WAIT_FOREVER);
	ptr = calloc_ex(nelem, elem_size, mp);
	mutex_give(mem_mutex.id);

	return ptr;
}
