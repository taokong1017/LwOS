#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <types.h>

struct ring_buffer {
	int32_t put_head;
	int32_t put_tail;
	int32_t put_base;
	int32_t get_head;
	int32_t get_tail;
	int32_t get_base;
	uint32_t size;
	uint8_t *buffer;
};

/* define and initialize a ring buffer */
#define ring_buffer_define(name, bytes)                                        \
	uint8_t ring_buffer_data_##name[bytes];                                    \
	struct ring_bufer name = {.buffer = ring_buffer_data_##name, .size = bytes}
#define ring_buffer_define_pow2(name, pow) ring_buffer_define(name, BIT(pow))
void ring_buffer_init(struct ring_buffer *buf, uint32_t size, uint8_t *data);
void ring_buffer_reset(struct ring_buffer *buf);

/* obtain a ring buffer status */
bool ring_buffer_is_empty(struct ring_buffer *buf);
uint32_t ring_buffer_free_size_get(struct ring_buffer *buf);
uint32_t ring_buffer_capacity_get(struct ring_buffer *buf);
uint32_t ring_buffer_used_size_get(struct ring_buffer *buf);

/* put data to a ring buffer */
uint32_t ring_buffer_put_claim(struct ring_buffer *buf, uint8_t **data,
							   uint32_t size);
bool ring_buffer_put_finish(struct ring_buffer *buf, uint32_t size);
uint32_t ring_buffer_put(struct ring_buffer *buf, const uint8_t *data,
						 uint32_t size);

/* get data from a ring buffer */
uint32_t ring_buffer_get_claim(struct ring_buffer *buf, uint8_t **data,
							   uint32_t size);
bool ring_buffer_get_finish(struct ring_buffer *buf, uint32_t size);
uint32_t ring_buffer_get(struct ring_buffer *buf, uint8_t *data, uint32_t size);
uint32_t ring_buffer_peek(struct ring_buffer *buf, uint8_t *data,
						  uint32_t size);

#endif
