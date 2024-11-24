#include <shell.h>
#include <string.h>
#include <menuconfig.h>
#include <ctype.h>
#include <limits.h>
#include <arch_atomic.h>

#define TAB_SPACES "  "
#define SHELL_MSG_TOO_MANY_ARGS "Too many arguments in the command.\n"
#define ASCII_MAX_CHAR 127
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define in_rage(val, min, max) ((val) >= (min) && (val) <= (max))
#define is_valid_assic_char(c) (in_rage(c, 0, ASCII_MAX_CHAR))

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

void shell_line_hexdump(struct shell *shell, int32_t offset, const char *data,
						uint32_t len) {
	int32_t index = 0;
	char ch = 0;

	shell_color_show(shell, SHELL_NORMAL, "%08X: ", offset);

	for (index = 0; index < CONFIG_SHELL_HEXDUMP_BYTES_PER_LINE; index++) {
		if (index > 0 && !(index % 8)) {
			shell_color_show(shell, SHELL_NORMAL, " ");
		}

		if (index < len) {
			shell_color_show(shell, SHELL_NORMAL, "%02x ", data[index] & 0xFF);
		} else {
			shell_color_show(shell, SHELL_NORMAL, "   ");
		}
	}

	shell_color_show(shell, SHELL_NORMAL, "|");

	for (index = 0; index < CONFIG_SHELL_HEXDUMP_BYTES_PER_LINE; index++) {
		if (index > 0 && !(index % 8)) {
			shell_color_show(shell, SHELL_NORMAL, " ");
		}

		if (index < len) {
			ch = data[index];

			shell_color_show(shell, SHELL_NORMAL, "%c",
							 isprint((int)ch) != 0 ? ch : '.');
		} else {
			shell_color_show(shell, SHELL_NORMAL, " ");
		}
	}

	shell_color_show(shell, SHELL_NORMAL, "|\n");
}

void shell_hexdump(struct shell *shell, const char *data, size_t len) {
	const char *ptr = data;
	size_t line_len;

	while (len) {
		line_len = min(len, CONFIG_SHELL_HEXDUMP_BYTES_PER_LINE);

		shell_line_hexdump(shell, ptr - data, ptr, line_len);

		len -= line_len;
		ptr += line_len;
	}
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

	*longest = 0;
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

static int32_t shell_str_common(const char *s1, const char *s2, int32_t n) {
	int32_t common = 0;

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

void shell_alt_metakeys_handle(struct shell *shell, char data) {
	if (data == SHELL_VT100_ASCII_ALT_B) {
		shell_op_cursor_word_move(shell, -1);
	} else if (data == SHELL_VT100_ASCII_ALT_F) {
		shell_op_cursor_word_move(shell, 1);
	}
}

void shell_ctrl_metakeys_handle(struct shell *shell, char data) {
	switch (data) {
	case SHELL_VT100_ASCII_CTRL_A: /* CTRL + A */
		shell_op_cursor_home_move(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_B: /* CTRL + B */
		shell_op_cursor_left_arrow(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_C: /* CTRL + C */
		shell_op_cursor_end_move(shell);
		if (!shell_cursor_is_in_empty_line(shell)) {
			shell_cursor_next_line_move(shell);
		}
		shell_state_set(shell, SHELL_STATE_ACTIVE);
		break;
	case SHELL_VT100_ASCII_CTRL_D: /* CTRL + D */
		shell_op_char_delete(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_E: /* CTRL + E */
		shell_op_cursor_end_move(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_F: /* CTRL + F */
		shell_op_cursor_right_arrow(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_K: /* CTRL + K */
		shell_op_cursor_from_delete(shell);
		break;
	case SHELL_VT100_ASCII_CTRL_L: /* CTRL + L */
		shell_show(shell, SHELL_VT100_CURSORHOME);
		shell_show(shell, SHELL_VT100_CLEARSCREEN);
		shell_prompt_and_cmd_print(shell);
		break;

	case SHELL_VT100_ASCII_CTRL_N: /* CTRL + N */
		shell_history_handle(shell, false);
		break;

	case SHELL_VT100_ASCII_CTRL_P: /* CTRL + P */
		shell_history_handle(shell, true);
		break;
	}
}

static errno_t shell_cmd_do_execute(struct shell *shell, int32_t argc,
									char **argv,
									const struct shell_entry *help_entry) {
	int32_t ret = 0;
	uint8_t mandatory = 0;
	uint8_t optional = 0;

	if (shell->shell_context->active_cmd.handler == NULL) {
		if (help_entry != NULL) {
			if (help_entry->help == NULL) {
				return ERRNO_SHELL_NO_EXEC;
			}
			if (help_entry->help != shell->shell_context->active_cmd.help) {
				shell->shell_context->active_cmd = *help_entry;
			}
			shell_internal_help_print(shell);
			return ERRNO_SHELL_HELP_PRINT;
		} else {
			return ERRNO_SHELL_NO_EXEC;
		}
	}

	if (shell->shell_context->active_cmd.args.mandatory) {
		mandatory = shell->shell_context->active_cmd.args.mandatory;
		optional = shell->shell_context->active_cmd.args.optional;
		ret = in_rage(argc, mandatory, (mandatory + optional));
	}

	if (!ret) {
		ret = shell->shell_context->active_cmd.handler(shell, argc,
													   (char **)argv);
	}

	return ret;
}

static void active_cmd_prepare(struct shell_entry *entry,
							   struct shell_entry *active_cmd,
							   struct shell_entry *help_entry, uint32_t *level,
							   uint32_t *handler_level, uint32_t *args_left) {
	if (entry->handler) {
		*active_cmd = *entry;
		*handler_level = *level;
		if (entry->subcmd == NULL) {
			*args_left = entry->args.mandatory - 1;
		}
	}

	if (entry->help) {
		*help_entry = *entry;
	}
}

errno_t shell_cmd_interal_execute(struct shell *shell) {
	struct shell_entry *entry = NULL;
	struct shell_entry help_entry = {.help = NULL};
	struct shell_entry *parent = NULL;
	enum shell_wildcard_status status;
	bool wildcard_found = false;
	bool has_last_handler = false;
	char *argv[CONFIG_SHELL_ARGC_MAX + 1] = {0};
	char **argvp = &argv[0];
	char *cmd_buf = shell->shell_context->cmd_buffer;
	char quote = 0;
	uint32_t cmd_level = 0;
	uint32_t cmd_with_handler_level = 0;
	uint32_t args_left = U32_MAX;
	int32_t argc = 0;

	shell_op_cursor_end_move(shell);
	if (!shell_cursor_is_in_empty_line(shell)) {
		shell_cursor_next_line_move(shell);
	}

	memset(&shell->shell_context->active_cmd, 0,
		   sizeof(shell->shell_context->active_cmd));
	shell_cmd_space_trim(shell);
	shell_history_add(shell->shell_history, shell->shell_context->cmd_buffer,
					  shell->shell_context->cmd_buffer_length);

	shell_wildcard_prepare(shell);

	while ((argc != 1) && (cmd_level < CONFIG_SHELL_ARGC_MAX) &&
		   args_left > 0) {
		quote = shell_make_argv(&argc, argvp, cmd_buf, 2);
		cmd_buf = (char *)argvp[1];

		if (argc == 0) {
			return -ERRNO_SHELL_NO_EXEC;
		} else if ((argc == 1) && (quote != 0)) {
			shell_color_show(shell, SHELL_ERROR, "not terminated: %c\n", quote);
			return -ERRNO_SHELL_NO_EXEC;
		}

		if (cmd_level > 0) {
			if (shell_help_cmd_is_request(argvp[0])) {
				if (help_entry.help) {
					shell->shell_context->active_cmd = help_entry;
					shell_internal_help_print(shell);
					return ERRNO_SHELL_HELP_PRINT;
				}
			}
			status = shell_wildcard_process(shell, entry, argvp[0]);
			if (status == SHELL_WILDCARD_NO_MATCH_FOUND) {
				break;
			}
			if (status != SHELL_WILDCARD_NOT_FOUND) {
				++cmd_level;
				wildcard_found = true;
				continue;
			}
		}

		if (has_last_handler == false) {
			entry = (struct shell_entry *)shell_cmd_find(parent, argvp[0]);
		}

		argvp++;
		args_left--;

		if (entry) {
			active_cmd_prepare(entry, &shell->shell_context->active_cmd,
							   &help_entry, &cmd_level, &cmd_with_handler_level,
							   &args_left);
			parent = entry;
		} else {
			has_last_handler = true;
		}

		if (args_left || (argc == 2)) {
			cmd_level++;
		}
	}

	if ((cmd_level >= CONFIG_SHELL_ARGC_MAX) && (argc == 2)) {
		shell_color_show(shell, SHELL_ERROR, "%s\n", SHELL_MSG_TOO_MANY_ARGS);
		return ERRNO_SHELL_NO_EXEC;
	}

	if (wildcard_found) {
		shell_wildcard_finalize(shell);
	}

	return shell_cmd_do_execute(shell, argc - cmd_with_handler_level,
								&argv[cmd_with_handler_level], &help_entry);
}

errno_t shell_cmd_execute(struct shell *shell, const char *cmd) {
	uint32_t cmd_len = strlen(cmd);

	if (!cmd || !shell) {
		return ERRNO_SHELL_EMPTY_PTR;
	}

	if (cmd_len == 0) {
		return ERRNO_SHELL_EMPTY_CMD;
	}

	strcpy(shell->shell_context->cmd_buffer, cmd);
	shell->shell_context->cmd_buffer_length = cmd_len;
	shell->shell_context->cmd_buffer_position = cmd_len;
	shell_cmd_interal_execute(shell);
	shell_cmd_buffer_clear(shell);

	return OK;
}

static void shell_receive_state_change(struct shell *shell,
									   enum shell_receive_state state) {
	shell->shell_context->receive_state = state;
}

static bool shell_new_line_process(struct shell *shell, char data) {
	atomic_t last_nl_char = 0;

	if ((data != '\r') && (data != '\n')) {
		atomic_set(&shell->shell_context->last_nl_char, 0);
		return false;
	}

	last_nl_char = atomic_get(&shell->shell_context->last_nl_char);
	if ((last_nl_char == 0) || (data == last_nl_char)) {
		atomic_set(&shell->shell_context->last_nl_char, data);
		return true;
	}

	return false;
}

static void shell_state_process(struct shell *shell) {
	int32_t count = 0;
	char data;

	while (true) {
		count = shell->shell_transport->transport_ops->read(
			shell->shell_transport, &data, 1);
		if (count <= 0) {
			return;
		}

		if (!is_valid_assic_char(data)) {
			continue;
		}

		switch (shell->shell_context->receive_state) {
		case SHELL_RECEIVE_DEFAULT:
			if (shell_new_line_process(shell, data)) {
				if (!shell->shell_context->cmd_buffer_length) {
					shell_history_mode_exit(shell->shell_history);
					shell_cursor_next_line_move(shell);
				} else {
					shell_cmd_interal_execute(shell);
				}
				shell_state_set(shell, SHELL_STATE_ACTIVE);
				continue;
			}

			switch (data) {
			case SHELL_VT100_ASCII_ESC: /* ESCAPE */
				shell_receive_state_change(shell, SHELL_RECEIVE_ESC);
				break;
			case SHELL_VT100_ASCII_BSPACE: /* BACKSPACE */
			case SHELL_VT100_ASCII_DEL:	   /* DELETE */
				shell_op_char_backspace(shell);
				break;
			case '\t': /* TAB */
				shell_tab_handle(shell);
				break;
			case '\0':
				break;
			default:
				if (isprint((int)data) != 0) {
					shell_op_char_insert(shell, data);
				} else {
					shell_ctrl_metakeys_handle(shell, data);
				}
				break;
			}
			break;

		case SHELL_RECEIVE_ESC:
			if (data == '[') {
				shell_receive_state_change(shell, SHELL_RECEIVE_ESC_SEQ);
				break;
			} else {
				shell_alt_metakeys_handle(shell, data);
			}
			shell_receive_state_change(shell, SHELL_RECEIVE_DEFAULT);
			break;

		case SHELL_RECEIVE_ESC_SEQ:
			shell_receive_state_change(shell, SHELL_RECEIVE_DEFAULT);
			switch (data) {
			case 'A': /* UP arrow */
				shell_history_handle(shell, true);
				break;
			case 'B': /* DOWN arrow */
				shell_history_handle(shell, false);
				break;
			case 'C': /* RIGHT arrow */
				shell_op_cursor_right_arrow(shell);
				break;
			case 'D': /* LEFT arrow */
				shell_op_cursor_left_arrow(shell);
				break;
			case 'F': /* END Button */
				shell_op_cursor_end_move(shell);
				break;
			case 'H': /* HOME Button */
				shell_op_cursor_home_move(shell);
				break;
			}
			break;

		default:
			shell_receive_state_change(shell, SHELL_RECEIVE_DEFAULT);
			break;
		}
	}

	shell_transport_buffer_flush(shell);
}

void shell_entry(struct shell *shell) {
	errno_t ret = OK;

	while (true) {
		ret = sem_take(shell->shell_sem_id, SEM_WAIT_FOREVER);
		if (ret != OK) {
			continue;
		}

		switch (shell->shell_context->state) {
		case SHELL_STATE_UNINITIALIZED:
		case SHELL_STATE_INITIALIZED:
			break;

		case SHELL_STATE_ACTIVE:
			shell_state_process(shell);
			break;
		default:
			break;
		}
	}
}
