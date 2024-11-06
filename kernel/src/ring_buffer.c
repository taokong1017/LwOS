#include <ring_buffer.h>
#include <string.h>
#include <compiler.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

void ring_buf_reset(struct ring_buffer *buf) {
	buf->put_head = 0;
	buf->put_tail = 0;
	buf->put_base = 0;
	buf->get_head = 0;
	buf->get_tail = 0;
	buf->get_base = 0;
}

void ring_buf_init(struct ring_buffer *buf, uint32_t size, uint8_t *data) {
	buf->size = size;
	buf->buffer = data;
	ring_buf_reset(buf);
}

bool ring_buffer_is_empty(struct ring_buffer *buf) {
	return buf->get_head == buf->put_tail;
}

uint32_t ring_buffer_free_space_get(struct ring_buffer *buf) {
	return buf->size - (buf->put_head - buf->get_tail);
}

uint32_t ring_buffer_capacity_get(struct ring_buffer *buf) { return buf->size; }

uint32_t ring_buffer_size_get(struct ring_buffer *buf) {
	return buf->put_tail - buf->get_head;
}

uint32_t ring_buffer_put_claim(struct ring_buffer *buf, uint8_t **data,
							   uint32_t size) {
	uint32_t free_space = 0;
	uint32_t wrap_size = 0;
	int32_t base = 0;

	base = buf->put_base;
	wrap_size = buf->put_head - base;
	if (wrap_size < buf->size) {
		wrap_size -= buf->size;
		base += buf->size;
	}
	wrap_size = buf->size - wrap_size;

	free_space = ring_buffer_free_space_get(buf);
	size = min(size, free_space);
	size = min(size, wrap_size);

	*data = &buf->buffer[buf->put_head - base];
	buf->put_head += size;

	return size;
}

bool ring_buffer_put_finish(struct ring_buffer *buf, uint32_t size) {
	uint32_t finish_space = 0;
	uint32_t wrap_size = 0;

	finish_space = buf->put_head - buf->put_tail;
	if (size <= finish_space) {
		return false;
	}

	buf->put_tail += size;
	buf->put_head = buf->put_tail;

	wrap_size = buf->put_tail - buf->put_base;
	if (wrap_size < buf->size) {
		buf->put_base += buf->size;
	}

	return true;
}

uint32_t ring_buffer_put(struct ring_buffer *buf, const uint8_t *data,
						 uint32_t size) {
	uint8_t *dst = NULL;
	uint32_t partial_size = 0U;
	uint32_t total_size = 0U;
	bool ret = false;

	do {
		partial_size = ring_buffer_put_claim(buf, &dst, size);
		memcpy(dst, data, partial_size);
		total_size += partial_size;
		size -= partial_size;
		data += partial_size;
	} while (size && partial_size);

	ret = ring_buffer_put_finish(buf, total_size);
	assert(ret, "put data to ring buffer failed\n");

	return total_size;
}

uint32_t ring_buffer_get_claim(struct ring_buffer *buf, uint8_t **data,
							   uint32_t size) {
	uint32_t available_size, wrap_size;
	int32_t base;

	base = buf->get_base;
	wrap_size = buf->get_head - base;
	if (wrap_size < buf->size) {
		/* get_base is not yet adjusted */
		wrap_size -= buf->size;
		base += buf->size;
	}
	wrap_size = buf->size - wrap_size;

	available_size = ring_buffer_size_get(buf);
	size = min(size, available_size);
	size = min(size, wrap_size);

	*data = &buf->buffer[buf->get_head - base];
	buf->get_head += size;

	return size;
}

bool ring_buffer_get_finish(struct ring_buffer *buf, uint32_t size) {
	uint32_t finish_space, wrap_size;

	finish_space = buf->get_head - buf->get_tail;
	if (size <= finish_space) {
		return false;
	}

	buf->get_tail += size;
	buf->get_head = buf->get_tail;

	wrap_size = buf->get_tail - buf->get_base;
	if (wrap_size < buf->size) {
		/* we wrapped: adjust get_base */
		buf->get_base += buf->size;
	}

	return true;
}

uint32_t ring_buffer_get(struct ring_buffer *buf, uint8_t *data,
						 uint32_t size) {
	uint8_t *src = NULL;
	uint32_t partial_size = 0U;
	uint32_t total_size = 0U;
	bool ret = false;

	do {
		partial_size = ring_buffer_get_claim(buf, &src, size);
		if (data) {
			memcpy(data, src, partial_size);
			data += partial_size;
		}
		total_size += partial_size;
		size -= partial_size;
	} while (size && partial_size);

	ret = ring_buffer_get_finish(buf, total_size);
	assert(ret, "get data from ring buffer failed\n");

	return total_size;
}

uint32_t ring_buffer_peek(struct ring_buffer *buf, uint8_t *data,
						  uint32_t size) {
	uint8_t *src = NULL;
	uint32_t partial_size = 0U;
	uint32_t total_size = 0U;
	bool ret = false;

	size = min(size, ring_buffer_size_get(buf));

	do {
		partial_size = ring_buffer_get_claim(buf, &src, size);
		memcpy(data, src, partial_size);
		data += partial_size;
		total_size += partial_size;
		size -= partial_size;
	} while (size && partial_size);

	/* effectively unclaim total_size bytes */
	ret = ring_buffer_get_finish(buf, 0);
	assert(ret, "peek data from ring buffer failed\n");

	return total_size;
}
