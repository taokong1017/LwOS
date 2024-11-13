#ifndef __SHELL_HISTORY_H__
#define __SHELL_HISTORY_H__

#include <ring_buffer.h>
#include <list.h>

struct shell_history {
	struct ring_buffer ring_buf;
	struct list_head list;
	struct list_head *current;
};

enum shell_history_direction {
	SHELL_HISTORY_UP,
	SHELL_HISTORY_DOWN,
};

/* Shell history interface */
void shell_history_init(struct shell_history *history);
bool shell_history_is_active(struct shell_history *history);
void shell_history_mode_exit(struct shell_history *history);
void shell_history_add(struct shell_history *history, const uint8_t *cmd_line,
					   size_t len);
bool shell_history_get(struct shell_history *history, uint8_t *buffer,
					   size_t *len, enum shell_history_direction direction);

#endif
