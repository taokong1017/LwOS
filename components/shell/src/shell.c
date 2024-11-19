#include <shell.h>
#include <string.h>

void shell_show(struct shell *shell, const char *format, ...) {
	va_list args;

	if ((!shell) || (!shell->shell_printf)) {
		return;
	}

	va_start(args, format);
	shell_printf(shell->shell_printf, format, args);
	va_end(args);

	return;
}

void shell_color_show(struct shell *shell, enum shell_vt100_color color,
					  const char *format, ...) {
	va_list args;
	struct shell_vt100_colors old_color;

	shell_vt100_colors_save(shell, &old_color);
	shell_vt100_color_set(shell, color);
	va_start(args, format);
	shell_printf(shell->shell_printf, format, args);
	va_end(args);
	shell_vt100_colors_restore(shell, old_color);
}

static void shell_vt100_bgcolor_set(struct shell *shell,
									enum shell_vt100_color bgcolor) {
	if (bgcolor >= VT100_COLOR_END) {
		return;
	}

	if ((bgcolor == SHELL_NORMAL) ||
		(shell->shell_context->vt100_context.col.bgcol == bgcolor)) {
		return;
	}

	shell->shell_context->vt100_context.col.bgcol = bgcolor;
	shell_show(shell, "\e[403%dm", bgcolor);
}

void shell_vt100_colors_restore(struct shell *shell,
								const struct shell_vt100_colors color) {
	shell_vt100_color_set(shell, color.col);
	shell_vt100_bgcolor_set(shell, color.bgcol);
}

static void shell_cmd_buffer_clear(struct shell *shell) {
	shell->shell_context->cmd_buffer[0] = '\0';
	shell->shell_context->cmd_buffer_length = 0;
	shell->shell_context->cmd_buffer_position = 0;
}

void shell_internal_help_print(struct shell *shell) {
	shell_help_cmd_print(shell, &shell->shell_context->active_cmd);
	shell_help_subcmd_print(shell, &shell->shell_context->active_cmd,
							"Subcommands:\n");
}

void shell_wrong_cmd_print(struct shell *shell) {
	shell_color_show(shell, SHELL_ERROR, "%s: wrong parameter count\n",
					 shell->shell_context->active_cmd.syntax);
	shell_internal_help_print(shell);
}

void shell_state_set(struct shell *shell, enum shell_state state) {
	shell->shell_context->state = state;
	shell_cmd_buffer_clear(shell);
	shell_prompt_and_cmd_print(shell);
}

enum shell_state shell_state_get(const struct shell *shell) {
	return shell->shell_context->state;
}

struct shell_entry *shell_selected_cmd_get(const struct shell *shell) {
	return shell->shell_context->selected_cmd;
}

void shell_tab_item_print(struct shell *shell, const char *option,
						  uint16_t longest_option) {
	static const char *tab = "  ";
	uint16_t columns = 0;
	uint16_t diff = 0;

	/* Function initialization has been requested. */
	if (option == NULL) {
		shell->shell_context->vt100_context.printed_cmd = 0;
		return;
	}

	longest_option += strlen(tab);

	columns = ((shell->shell_context->vt100_context.cons.terminal_width -
				strlen(tab)) /
			   longest_option);
	diff = longest_option - strlen(option);

	if (shell->shell_context->vt100_context.printed_cmd++ % columns == 0U) {
		shell_color_show(shell, SHELL_OPTION, "\n%s%s", tab, option);
	} else {
		shell_color_show(shell, SHELL_OPTION, "%s", option);
	}

	shell_op_cursor_horiz_move(shell, diff);
}

void history_handle(struct shell *shell, bool up) {
	size_t cmd_len = 0;
	bool history_mode = false;

	if (!shell_history_is_active(shell->shell_history)) {
		if (up) {
			cmd_len = strlen(shell->shell_context->cmd_buffer);
			if (cmd_len) {
				strncpy(shell->shell_context->temp_buffer,
						shell->shell_context->cmd_buffer,
						CONFIG_SHELL_CMD_BUFFER_SIZE);
			} else {
				shell->shell_context->temp_buffer[0] = '\0';
			}
		} else {
			return;
		}
	}

	history_mode = shell_history_get(shell->shell_history,
									 shell->shell_context->cmd_buffer, &cmd_len,
									 SHELL_HISTORY_UP);
	if (!history_mode) {
		strcpy(shell->shell_context->cmd_buffer,
			   shell->shell_context->temp_buffer);
		cmd_len = strlen(shell->shell_context->cmd_buffer);
	}

	shell_op_cursor_home_move(shell);
	shell_show(shell, SHELL_VT100_CLEAREOS);
	shell_cmd_print(shell);
	shell->shell_context->cmd_buffer_position = cmd_len;
	shell->shell_context->cmd_buffer_length = cmd_len;
	shell_cursor_next_line_move(shell);
}
