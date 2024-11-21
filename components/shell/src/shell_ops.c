#include <shell.h>
#include <shell_ops.h>
#include <string.h>
#include <ctype.h>

void shell_op_cursor_horiz_move(struct shell *shell, int32_t delta) {
	char dir = delta > 0 ? 'C' : 'D';

	if (delta == 0) {
		return;
	}

	if (delta < 0) {
		delta = -delta;
	}

	shell_show(shell, "\e[%d%c", delta, dir);
}

void shell_op_cursor_vert_move(struct shell *shell, int32_t delta) {
	char dir = delta > 0 ? 'A' : 'B';

	if (delta == 0) {
		return;
	}

	if (delta < 0) {
		delta = -delta;
	}

	shell_show(shell, "\e[%d%c", delta, dir);
}

bool shell_cmd_is_full_line(struct shell *shell) {
	return ((shell->shell_context->cmd_buffer_length +
			 strlen(shell->shell_context->cur_prompt)) %
				shell->shell_context->vt100_context.cons.terminal_width ==
			0U);
}

static void shell_prompt_print(struct shell *shell) {
	shell_color_show(shell, SHELL_INFO, "%s", shell->shell_context->cur_prompt);
}

void shell_cmd_print(struct shell *shell) {
	int beg_offset = 0;
	int end_offset = 0;
	int cmd_width = strlen(shell->shell_context->cmd_buffer);
	int adjust = shell->shell_context->vt100_context.cons.name_len;
	char ch = 0;

	while (cmd_width >
		   shell->shell_context->vt100_context.cons.terminal_width - adjust) {
		end_offset +=
			shell->shell_context->vt100_context.cons.terminal_width - adjust;
		ch = shell->shell_context->cmd_buffer[end_offset];
		shell->shell_context->cmd_buffer[end_offset] = '\0';

		shell_show(shell, "%s\n",
				   &shell->shell_context->cmd_buffer[beg_offset]);

		shell->shell_context->cmd_buffer[end_offset] = ch;
		cmd_width -=
			(shell->shell_context->vt100_context.cons.terminal_width - adjust);
		beg_offset = end_offset;
		adjust = 0;
	}

	if (cmd_width > 0) {
		shell_show(shell, "%s", &shell->shell_context->cmd_buffer[beg_offset]);
	}
}

void shell_prompt_and_cmd_print(struct shell *shell) {
	shell_prompt_print(shell);
	shell_cmd_print(shell);
	shell_op_cursor_position_synchronize(shell);
}

void shell_op_cursor_position_synchronize(struct shell *shell) {
	struct shell_multiline_cons *cons =
		&shell->shell_context->vt100_context.cons;
	bool last_line = false;

	shell_multiline_data_calc(cons, shell->shell_context->cmd_buffer_position,
							  shell->shell_context->cmd_buffer_length);
	last_line = (cons->cur_y == cons->cur_y_end);

	if (shell_cmd_is_full_line(shell)) {
		shell_cursor_next_line_move(shell);
	}

	if (last_line) {
		shell_op_cursor_horiz_move(shell, cons->cur_x - cons->cur_x_end);
	} else {
		shell_op_cursor_vert_move(shell, cons->cur_y_end - cons->cur_y);
		shell_op_cursor_horiz_move(shell, cons->cur_x - cons->cur_x_end);
	}
}

void shell_cursor_next_line_move(struct shell *shell) {
	shell_show(shell, "\n");
}

void shell_op_cursor_move(struct shell *shell, uint32_t val) {
	struct shell_multiline_cons *cons =
		&shell->shell_context->vt100_context.cons;
	uint32_t new_pos = shell->shell_context->cmd_buffer_position + val;
	int32_t row_span = 0;
	int32_t col_span = 0;

	shell_multiline_data_calc(cons, shell->shell_context->cmd_buffer_position,
							  shell->shell_context->cmd_buffer_length);
	/* Calculate the new cursor. */
	row_span = shell_row_span_with_buffer_offsets_get(
		&shell->shell_context->vt100_context.cons,
		shell->shell_context->cmd_buffer_position, new_pos);
	col_span = shell_column_span_with_buffer_offsets_get(
		&shell->shell_context->vt100_context.cons,
		shell->shell_context->cmd_buffer_position, new_pos);
	shell_op_cursor_vert_move(shell, -row_span);
	shell_op_cursor_horiz_move(shell, col_span);
	shell->shell_context->cmd_buffer_position = new_pos;
}

void shell_op_cursor_home_move(struct shell *shell) {
	shell_op_cursor_move(shell, -shell->shell_context->cmd_buffer_position);
}

void shell_op_cursor_end_move(struct shell *shell) {
	shell_op_cursor_move(shell, shell->shell_context->cmd_buffer_length -
									shell->shell_context->cmd_buffer_position);
}

static uint32_t shell_shift_calc(const char *str, uint32_t pos, uint32_t len,
								 int32_t sign) {
	bool found = false;
	uint32_t ret = 0;
	uint32_t index = 0;

	while (true) {
		index = pos + ret * sign;
		if (((index == 0U) && (sign < 0)) || ((index == len) && (sign > 0))) {
			break;
		}
		if (isalnum((int)str[index]) != 0) {
			found = true;
		} else {
			if (found) {
				break;
			}
		}
		ret++;
	}

	return ret;
}

void shell_op_cursor_word_move(struct shell *shell, int32_t val) {
	int32_t shift = 0;
	int32_t sign = 0;

	if (val < 0) {
		val = -val;
		sign = -1;
	} else {
		sign = 1;
	}

	while (val--) {
		shift = shell_shift_calc(shell->shell_context->cmd_buffer,
								 shell->shell_context->cmd_buffer_position,
								 shell->shell_context->cmd_buffer_length, sign);
		shell_op_cursor_move(shell, sign * shift);
	}
}

static void reprint_from_cursor(struct shell *shell, uint32_t diff,
								bool data_removed) {
	if (data_removed) {
		shell_show(shell, SHELL_VT100_CLEAREOS);
	}

	shell_color_show(
		shell, SHELL_NORMAL, "%s",
		&shell->shell_context
			 ->cmd_buffer[shell->shell_context->cmd_buffer_position]);
	shell->shell_context->cmd_buffer_position =
		shell->shell_context->cmd_buffer_length;

	if (shell_cmd_is_full_line(shell)) {
		if (((data_removed) && (diff > 0)) || (!data_removed)) {
			shell_cursor_next_line_move(shell);
		}
	}

	shell_op_cursor_move(shell, -diff);
}

static void shell_data_insert(struct shell *shell, const char *data,
							  uint32_t len) {
	uint32_t after = shell->shell_context->cmd_buffer_length -
					 shell->shell_context->cmd_buffer_position;
	char *curr_pos =
		&shell->shell_context
			 ->cmd_buffer[shell->shell_context->cmd_buffer_position];

	if ((shell->shell_context->cmd_buffer_length + len) >=
		CONFIG_SHELL_CMD_BUFFER_SIZE) {
		return;
	}

	memmove(curr_pos + len, curr_pos, after);
	memcpy(curr_pos, data, len);
	shell->shell_context->cmd_buffer_length += len;
	shell->shell_context->cmd_buffer[shell->shell_context->cmd_buffer_length] =
		'\0';
	reprint_from_cursor(shell, after, false);
}

void shell_op_completion_insert(struct shell *shell, const char * compl,
								uint32_t compl_len) {
	shell_data_insert(shell, compl, compl_len);
}

void shell_op_char_insert(struct shell *shell, char data) {
	shell_data_insert(shell, &data, 1);
}

void shell_vt100_color_set(struct shell *shell, enum shell_vt100_color color) {
	if (color >= VT100_COLOR_END) {
		return;
	}

	if (shell->shell_context->vt100_context.col.col == color) {
		return;
	}

	shell->shell_context->vt100_context.col.col = color;

	if (color != SHELL_NORMAL) {
		shell_show(shell, "\e[1;3%dm", color);
	} else {
		shell_show(shell, SHELL_VT100_MODESOFF);
	}
}

void shell_vt100_colors_save(const struct shell *shell,
							 struct shell_vt100_colors *color) {
	memcpy(color, &shell->shell_context->vt100_context.col, sizeof(*color));
}
