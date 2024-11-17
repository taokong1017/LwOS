#include <string.h>
#include <shell_history.h>

/*
 * history item layout as follows:
 * *********************************************************************
 *  ******** history list front ********
 *  |         ---------------          ^
 *  |        |    command3   |         |
 *  |     |   ---------------   ^      |
 *  |     |   ---------------   |      |
 *  |     |  |    command2   |  |      |
 *  | next|   ---------------   | prev |
 *  |     |   ---------------   |      |
 *  |     v  |    command1   |  |      |
 *  |         ---------------          |
 *  |        |    command0   |         |
 *  V         ---------------          |
 *  down                               up
 *  ******** history list end *********
 * *********************************************************************
 */

#define cal_padding_bytes(total_len) ((~(total_len) + 1) & (sizeof(void *) - 1))
#define invalid_dirction(direction)                                            \
	((direction != SHELL_HISTORY_UP) && (direction != SHELL_HISTORY_DOWN))

struct shell_history_item {
	struct list_head inode;
	uint32_t len;
	uint32_t padding;
	char data[0];
};

void shell_history_init(struct shell_history *history) {
	INIT_LIST_HEAD(&history->list);
	history->current = NULL;
}

bool shell_history_is_active(struct shell_history *history) {
	return history->current == NULL ? false : true;
}

void shell_history_mode_exit(struct shell_history *history) {
	history->current = NULL;
}

static void shell_history_item_add(struct shell_history *history,
								   struct shell_history_item *item,
								   const char *src, size_t len,
								   uint32_t padding) {
	item->len = len;
	item->padding = padding;
	memcpy(item->data, src, len);
	list_add_tail(&item->inode, &history->list);
}

static void shell_history_oldest_item_remove(struct shell_history *history) {
	struct shell_history_item *h_item = NULL;
	uint32_t total_len = 0;

	if (list_empty(&history->list)) {
		return;
	}

	h_item = list_first_entry(&history->list, struct shell_history_item, inode);
	total_len = offsetof(struct shell_history_item, data) + h_item->len +
				h_item->padding;
	ring_buffer_get(&history->ring_buf, NULL, total_len);
	list_del_init(history->list.next);
}

void shell_history_add(struct shell_history *history, const char *cmd_line,
					   size_t len) {
	uint32_t total_len = len + offsetof(struct shell_history_item, data);
	uint32_t padding = cal_padding_bytes(total_len);
	struct shell_history_item *last_item = NULL;
	struct shell_history_item *h_item = NULL;
	struct shell_history_item *h_prev_item = NULL;
	uint32_t claim_len = 0;
	uint32_t claim_len2 = 0;

	total_len += padding;
	shell_history_mode_exit(history);

	if ((total_len > ring_buffer_capacity_get(&history->ring_buf)) ||
		(len == 0)) {
		return;
	}

	/* Check if the command line is the latest one in the history */
	if (!list_empty(&history->list)) {
		last_item =
			list_last_entry(&history->list, struct shell_history_item, inode);
		if ((last_item->len == len) &&
			(memcmp(last_item->data, cmd_line, len) == 0)) {
			return;
		}
	}

	do {
		/* Reset the history ring buffer */
		if (ring_buffer_is_empty(&history->ring_buf)) {
			ring_buf_reset(&history->ring_buf);
		}

		/*
		 * Attempt to access the free ring buffer space for history command item
		 */
		claim_len = ring_buffer_put_claim(&history->ring_buf,
										  (uint8_t **)&h_item, total_len);
		if (claim_len < total_len) {
			claim_len2 = ring_buffer_put_claim(
				&history->ring_buf, (uint8_t **)&h_prev_item, total_len);
			if (claim_len2 == total_len) {
				last_item->padding += claim_len;
				total_len += claim_len;
				claim_len = total_len;
			}
		}

		/* Add command item to the history list */
		if (claim_len == total_len) {
			shell_history_item_add(history, h_item, cmd_line, len, padding);
			ring_buffer_put_finish(&history->ring_buf, claim_len);
			break;
		}

		/* Need to remove the odlest command from the history list */
		ring_buffer_put_finish(&history->ring_buf, 0);
		shell_history_oldest_item_remove(history);
	} while (true);

	return;
}

bool shell_history_get(struct shell_history *history, char *buffer,
					   size_t *len, enum shell_history_direction direction) {
	struct shell_history_item *h_item = NULL;
	struct list_head *logical_next = NULL;

	if (list_empty(&history->list) || invalid_dirction(direction)) {
		return false;
	}

	if (direction == SHELL_HISTORY_DOWN) {
		if (history->current == NULL) {
			*len = 0;
			return false;
		}
		logical_next = history->current->next;
		h_item = list_entry(logical_next, struct shell_history_item, inode);
	} else {
		if (history->current == NULL) {
			logical_next = history->list.prev;
			h_item = list_entry(logical_next, struct shell_history_item, inode);
		} else {
			logical_next = history->current->prev;
			h_item = list_entry(logical_next, struct shell_history_item, inode);
		}
	}

	if (logical_next != &history->list) {
		history->current = &h_item->inode;
		memcpy(buffer, h_item->data, h_item->len);
		*len = h_item->len;
		buffer[*len] = '\0';
		return true;
	}

	return false;
}