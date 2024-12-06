#ifndef __SHELL_HISTORY_H__
#define __SHELL_HISTORY_H__

#include <ring_buffer.h>
#include <list.h>

struct shell_history {
	struct ring_buffer *ring_buffer;
	struct list_head list;
	struct list_head *current;
};

enum shell_history_direction {
	SHELL_HISTORY_UP,
	SHELL_HISTORY_DOWN,
};

#define shell_history_define(name, buffer_size)                                \
	static uint8_t __attribute__((aligned(sizeof(void *))))                    \
		name##_ring_buf_data[buffer_size];                                     \
	static struct ring_buffer name##_ring_buffer = {                           \
		.size = buffer_size,                                                   \
		.buffer = name##_ring_buf_data,                                        \
	};                                                                         \
	static struct shell_history name##_history = {                             \
		.ring_buffer = &name##_ring_buffer,                                    \
		.current = NULL,                                                       \
	};

/* Shell history interface */
void shell_history_init(struct shell_history *history);
bool shell_history_is_active(struct shell_history *history);
void shell_history_mode_exit(struct shell_history *history);
void shell_history_add(struct shell_history *history, const char *cmd_line,
					   size_t len);
bool shell_history_get(struct shell_history *history, char *buffer, size_t *len,
					   enum shell_history_direction direction);

#endif
