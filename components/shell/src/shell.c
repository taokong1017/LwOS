#include <shell.h>
#include <string.h>
#include <menuconfig.h>
#include <ctype.h>
#include <limits.h>

#define TAB_SPACES "  "
#define max(a, b) ((a) > (b) ? (a) : (b))

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

void shell_tab_item_print(struct shell *shell, const char *option,
						  uint16_t longest_option) {
	static const char *tab = TAB_SPACES;
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

static uint32_t completion_space_get(const struct shell *shell) {
	uint32_t space = (CONFIG_SHELL_CMD_BUFFER_SIZE - 1) -
					 shell->shell_context->cmd_buffer_length;

	return space;
}

static bool shell_tab_prepare(struct shell *shell,
							  const struct shell_entry **cmd, char ***argv,
							  int32_t *argc, int32_t *complete_arg_idx) {
	uint32_t space = completion_space_get(shell);
	int32_t search_argc = 0;

	if (space == 0) {
		return false;
	}

	memcpy(shell->shell_context->temp_buffer, shell->shell_context->cmd_buffer,
		   shell->shell_context->cmd_buffer_position);
	shell->shell_context
		->temp_buffer[shell->shell_context->cmd_buffer_position] = '\0';

	shell_make_argv(argc, *argv, shell->shell_context->temp_buffer,
					CONFIG_SHELL_ARGC_MAX);
	if (*argc > CONFIG_SHELL_ARGC_MAX) {
		return false;
	}

	(*argv)[*argc] = NULL;
	space = (shell->shell_context->cmd_buffer_position > 0)
				? isspace((int)shell->shell_context->cmd_buffer
							  [shell->shell_context->cmd_buffer_position - 1])
				: 0;
	search_argc = space ? *argc : *argc - 1;
	*cmd = shell_get_last_command(NULL, search_argc, *argv, complete_arg_idx);

	if ((*cmd == NULL) && (search_argc != 0)) {
		return false;
	}

	return true;
}

static bool shell_is_completion_candidate(const char *candidate,
										  const char *str, uint32_t len) {
	return (strncmp(candidate, str, len) == 0) ? true : false;
}

static void shell_completion_candidates_find(
	struct shell *shell, const struct shell_entry *cmd, const char *incompl_cmd,
	uint32_t *first_index, uint32_t *count, uint32_t *longest) {
	const struct shell_entry *candidate = NULL;
	uint32_t incompl_cmd_len = strlen(incompl_cmd);
	uint32_t index = 0;

	*longest = 0U;
	*count = 0;

	while ((candidate = shell_cmd_get(cmd, index)) != NULL) {
		bool is_candidate;
		is_candidate = shell_is_completion_candidate(
			candidate->syntax, incompl_cmd, incompl_cmd_len);
		if (is_candidate) {
			*longest = max(strlen(candidate->syntax), *longest);
			if (*count == 0) {
				*first_index = index;
			}
			(*count)++;
		}

		index++;
	}
}

static void autocomplete(struct shell *shell, const struct shell_entry *cmd,
						 const char *arg, uint32_t subcmd_index) {
	const struct shell_entry *match = NULL;
	uint32_t cmd_len = 0;
	uint32_t arg_len = strlen(arg);

	match = shell_cmd_get(cmd, subcmd_index);
	cmd_len = strlen(match->syntax);

	if (cmd_len != arg_len) {
		shell_op_completion_insert(shell, match->syntax + arg_len,
								   cmd_len - arg_len);
	}

	if (isspace((int)shell->shell_context
					->cmd_buffer[shell->shell_context->cmd_buffer_position]) ==
		0) {
		shell_op_char_insert(shell, ' ');
	} else {
		shell_op_cursor_move(shell, 1);
	}
}

static void shell_tab_options_print(struct shell *shell,
									const struct shell_entry *cmd,
									const char *str, uint32_t first,
									uint32_t count, uint32_t longest) {
	const struct shell_entry *match;
	uint32_t str_len = strlen(str);
	uint32_t index = first;

	while (count) {
		match = shell_cmd_get(cmd, index);
		index++;
		if (str && match->syntax &&
			!shell_is_completion_candidate(match->syntax, str, str_len)) {
			continue;
		}

		shell_tab_item_print(shell, match->syntax, longest);
		count--;
	}

	shell_cursor_next_line_move(shell);
	shell_prompt_and_cmd_print(shell);
}

static size_t shell_str_common(const char *s1, const char *s2, size_t n) {
	size_t common = 0;

	while ((n > 0) && (*s1 == *s2) && (*s1 != '\0')) {
		s1++;
		s2++;
		n--;
		common++;
	}

	return common;
}

static uint16_t common_beginning_find(struct shell *shell,
									  const struct shell_entry *cmd,
									  const char **str, uint32_t first,
									  uint32_t count, uint16_t arg_len) {
	const struct shell_entry *match = NULL;
	const struct shell_entry *match2 = NULL;
	uint16_t common = U16_MAX;
	size_t index = first + 1;
	int32_t curr_common = 0;

	match = shell_cmd_get(cmd, first);
	strncpy(shell->shell_context->temp_buffer, match->syntax,
			sizeof(shell->shell_context->temp_buffer) - 1);

	*str = match->syntax;

	while (count > 1) {
		match2 = shell_cmd_get(cmd, index++);
		if (match2 == NULL) {
			break;
		}

		curr_common = shell_str_common(shell->shell_context->temp_buffer,
									   match2->syntax, U16_MAX);
		if ((arg_len == 0U) || (curr_common >= arg_len)) {
			--count;
			common = (curr_common < common) ? curr_common : common;
		}
	}

	return common;
}

static void shell_partial_autocomplete(struct shell *shell,
									   const struct shell_entry *cmd,
									   const char *arg, uint32_t first,
									   uint32_t count) {
	const char *completion = NULL;
	uint32_t arg_len = strlen(arg);
	uint16_t common =
		common_beginning_find(shell, cmd, &completion, first, count, arg_len);

	if (common) {
		shell_op_completion_insert(shell, &completion[arg_len],
								   common - arg_len);
	}
}

void shell_history_handle(struct shell *shell, bool up) {
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

void shell_tab_handle(struct shell *shell) {
	char *argv[CONFIG_SHELL_ARGC_MAX + 1] = {NULL};
	const struct shell_entry *cmd = NULL;
	bool tab_possible = false;
	int32_t arg_index = 0;
	int32_t argc = 0;
	uint32_t first = 0;
	uint32_t count = 0;
	uint32_t longest = 0;

	tab_possible =
		shell_tab_prepare(shell, &cmd, (char ***)&argv, &argc, &arg_index);
	if (!tab_possible) {
		return;
	}

	shell_completion_candidates_find(shell, cmd, argv[arg_index], &first,
									 &count, &longest);
	if (count == 1) {
		autocomplete(shell, cmd, argv[arg_index], first);
	} else if (count > 1) {
		shell_tab_options_print(shell, cmd, argv[arg_index], first, count,
								longest);
		shell_partial_autocomplete(shell, cmd, argv[arg_index], first, count);
	}
}
