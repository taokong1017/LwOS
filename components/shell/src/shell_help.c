#include <string.h>
#include <ctype.h>
#include <shell.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

static void shell_formatted_text_print(struct shell *shell, const char *str,
									   size_t terminal_offset,
									   bool offset_first_line) {
	size_t offset = 0;
	size_t length = 0;
	size_t index = 0;

	if (str == NULL) {
		return;
	}

	if (offset_first_line) {
		shell_op_cursor_horiz_move(shell, terminal_offset);
	}

	/* Skipping whitespace */
	while (isspace((int)(*(str + offset))) != 0) {
		++offset;
	}

	while (true) {
		index = 0;
		length = strlen(str) - offset;
		if (length <= shell->shell_context->vt100_context.cons.terminal_width -
						  terminal_offset) {
			for (index = 0; index < length; index++) {
				if (*(str + offset + index) == '\n') {
					shell_transport_buffer_flush(shell);
					shell_show(shell, str + offset, index);
					offset += index + 1;
					shell_cursor_next_line_move(shell);
					shell_op_cursor_horiz_move(shell, terminal_offset);
					break;
				}
			}

			shell_show(shell, str + offset);
			break;
		}

		/*
		 * String is longer than terminal line so text needs to
		 * divide in the way to not divide words.
		 */
		length = shell->shell_context->vt100_context.cons.terminal_width -
				 terminal_offset;

		while (true) {
			/* Determining line break. */
			if (isspace((int)(*(str + offset + index))) != 0) {
				length = index;
				if (*(str + offset + index) == '\n') {
					break;
				}
			}

			if ((index + terminal_offset) >=
				shell->shell_context->vt100_context.cons.terminal_width) {
				/* End of line reached. */
				break;
			}

			++index;
		}

		/*
		 * Writing one line, fprintf IO buffer must be flushed
		 * before calling shell_write.
		 */
		shell_transport_buffer_flush(shell);
		shell_show(shell, str + offset, length);
		offset += length;

		/* Calculating text offset to ensure that next line will
		 * not begin with a space.
		 */
		while (isspace((int)(*(str + offset))) != 0) {
			++offset;
		}

		shell_cursor_next_line_move(shell);
		shell_op_cursor_horiz_move(shell, terminal_offset);
	}
}

void shell_help_cmd_print(struct shell *shell,
						  const struct shell_entry *entry) {
	static const char cmd_sep[] = " - "; /* commands separator */
	uint16_t field_width = 0;

	field_width = strlen(entry->syntax) + strlen(cmd_sep);

	shell_show(shell, "%s%s", entry->syntax, cmd_sep);

	shell_formatted_text_print(shell, entry->help, field_width, false);
}

static void help_item_print(struct shell *shell, const char *item_name,
							uint32_t item_name_width, const char *item_help) {
	static const char tabulator[] = "  ";
	uint32_t offset = 2 * strlen(tabulator) + item_name_width + 1;
	uint32_t space_num = 0;
	uint32_t index = 0;
	const char space = ' ';

	if ((item_name == NULL) || (item_name[0] == '\0')) {
		return;
	}

	shell_color_show(shell, SHELL_NORMAL, "%s%s", tabulator, item_name);

	space_num = item_name_width - strlen(item_name);
	if (item_help) {
		for (index = 0; index < space_num; index++) {
			shell_show(shell, "%c", space);
		}
	}

	if (item_help == NULL) {
		shell_cursor_next_line_move(shell);
		return;
	} else {
		shell_color_show(shell, SHELL_NORMAL, "%s: ", tabulator);
	}

	shell_formatted_text_print(shell, item_help, offset, false);

	return;
}

void shell_help_subcmd_print(struct shell *shell,
							 const struct shell_entry *parent,
							 const char *description) {
	const struct shell_entry *entry = NULL;
	uint32_t max_len = 0U;
	uint32_t index = 0;

	/* Search for the longest subcommand to print */
	while ((entry = shell_cmd_get(parent, index++)) != NULL) {
		max_len = max(max_len, strlen(entry->syntax));
	}

	if (max_len == 0U) {
		return;
	}

	if (description != NULL) {
		shell_color_show(shell, SHELL_NORMAL, description);
	}

	/* Print subcommands and help string */
	index = 0;
	while ((entry = shell_cmd_get(parent, index++)) != NULL) {
		help_item_print(shell, entry->syntax, max_len, entry->help);
	}

	return;
}

bool shell_help_cmd_is_request(const char *str) {
	if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
		return true;
	}

	return false;
}
