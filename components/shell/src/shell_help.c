#include <string.h>
#include <ctype.h>
#include <shell_types.h>
#include <shell_help.h>
#include <shell_ops.h>
#include <shell_util.h>

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
		if (length <= shell->shell_context->vt100_context.cons.terminal_wid -
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
		length = shell->shell_context->vt100_context.cons.terminal_wid -
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
				shell->shell_context->vt100_context.cons.terminal_wid) {
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

bool shell_help_cmd_is_request(const char *str) {
	if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
		return true;
	}

	return false;
}
