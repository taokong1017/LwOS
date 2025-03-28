#include <shell.h>
#include <iterable_section.h>
#include <string.h>
#include <ctype.h>

void shell_transport_buffer_flush(struct shell *shell) {
	shell_printf_flush(shell->shell_printf);
}

static uint32_t shell_root_cmd_count(void) {
	uint32_t len = 0;

	type_section_count(struct shell_cmd_entry, shell_root_cmd, &len);

	return len;
}

static const struct shell_cmd_entry *shell_root_cmd_get(uint32_t id) {
	struct shell_cmd_entry *cmd_entry;

	type_section_get(struct shell_cmd_entry, shell_root_cmd, id, &cmd_entry);

	return cmd_entry;
}

const struct shell_entry *shell_root_cmd_find(const char *syntax) {
	const uint32_t cmd_count = shell_root_cmd_count();
	const struct shell_cmd_entry *cmd_entry = NULL;
	uint32_t index = 0;

	for (index = 0; index < cmd_count; ++index) {
		cmd_entry = shell_root_cmd_get(index);
		if ((shell_strlen(syntax) == shell_strlen(cmd_entry->entry->syntax)) &&
			(strcmp(syntax, cmd_entry->entry->syntax) == 0)) {
			return cmd_entry->entry;
		}
	}

	return NULL;
}

static bool shell_is_section_subcmd(const struct shell_cmd_entry *cmd_entry) {
	type_section_start_extern(struct shell_cmd_entry, shell_sub_cmd);
	type_section_end_extern(struct shell_cmd_entry, shell_sub_cmd);

	return (cmd_entry >= type_section_start(shell_sub_cmd)) &&
		   (cmd_entry < type_section_end(shell_sub_cmd));
}

const struct shell_entry *shell_cmd_get(const struct shell_entry *parent,
										uint32_t index) {
	const struct shell_entry *entry = NULL;
	const struct shell_entry *entry_list = NULL;

	if (parent == NULL) {
		return (index < shell_root_cmd_count())
				   ? shell_root_cmd_get(index)->entry
				   : NULL;
	}

	if (shell_is_section_subcmd(parent->subcmd)) {
		entry_list = parent->subcmd->entry;
		if (entry_list[index].syntax != NULL) {
			entry = &entry_list[index];
		}
	}

	return entry;
}

const struct shell_entry *shell_cmd_find(const struct shell_entry *parent,
										 const char *cmd_str) {
	const struct shell_entry *entry;
	size_t index = 0;

	while ((entry = shell_cmd_get(parent, index++)) != NULL) {
		if ((shell_strlen(cmd_str) == shell_strlen(entry->syntax)) &&
			(strcmp(cmd_str, entry->syntax) == 0)) {
			return entry;
		}
	}

	return NULL;
}

static void shell_buffer_space_trim(char *buffer, size_t *length) {
	uint32_t i = 0;

	if (buffer[0] == '\0') {
		return;
	}

	while (isspace((int)buffer[*length - 1]) != 0) {
		*length -= 1;
		if (*length == 0) {
			buffer[0] = '\0';
			return;
		}
	}
	buffer[*length] = '\0';

	while (isspace((int)buffer[i++]) != 0) {
	}

	if (--i > 0) {
		memmove(buffer, buffer + i, (*length + 1) - i); /* +1 for '\0' */
		*length = *length - i;
	}
}

void shell_cmd_space_trim(struct shell *shell) {
	shell_buffer_space_trim(shell->shell_context->cmd_buffer,
							&shell->shell_context->cmd_buffer_length);
	shell->shell_context->cmd_buffer_position =
		shell->shell_context->cmd_buffer_length;
}

void shell_spaces_trim(char *str) {
	int32_t len = shell_strlen(str);
	int32_t shift = 0;
	int32_t i = 0;
	int32_t j = 0;

	if (!str || (len == 0)) {
		return;
	}

	for (i = 0; i < len - 1; i++) {
		if (isspace((int)str[i]) != 0) {
			for (j = i + 1; j < len; j++) {
				if (isspace((int)str[j]) != 0) {
					shift++;
					continue;
				}

				if (shift > 0) {
					memmove(&str[i + 1], &str[j], len - j + 1);
					len -= shift;
					shift = 0;
				}

				break;
			}
		}
	}
}

void shell_pattern_remove(char *buff, size_t *buff_len, const char *pattern) {
	char *pattern_addr = strstr(buff, pattern);
	uint32_t pattern_len = shell_strlen(pattern);
	uint32_t shift = 0;

	if (!pattern_addr) {
		return;
	}

	if (pattern_addr > buff) {
		if (*(pattern_addr - 1) == ' ') {
			pattern_len++;
			pattern_addr--;
		}
	}

	shift = shell_strlen(pattern_addr) - pattern_len + 1; /* +1 for EOS */
	*buff_len -= pattern_len;

	memmove(pattern_addr, pattern_addr + pattern_len, shift);
}

void shell_multiline_data_calc(struct shell_multiline_cons *cons,
							   uint32_t buff_pos, uint32_t buff_len) {
	cons->cur_x = (buff_pos + cons->name_len) % cons->terminal_width + 1;
	cons->cur_y = (buff_pos + cons->name_len) / cons->terminal_width + 1;
	cons->cur_y_end = (buff_len + cons->name_len) / cons->terminal_width + 1;
	cons->cur_x_end = (buff_len + cons->name_len) % cons->terminal_width + 1;
}

static uint32_t
shell_line_num_with_buffer_offset_get(struct shell_multiline_cons *cons,
									  uint32_t buffer_pos) {
	return ((buffer_pos + cons->name_len) / cons->terminal_width);
}

static uint32_t
shell_col_num_with_buffer_offset_get(struct shell_multiline_cons *cons,
									 uint32_t buffer_pos) {
	return (1 + ((buffer_pos + cons->name_len) % cons->terminal_width));
}

int32_t
shell_column_span_with_buffer_offsets_get(struct shell_multiline_cons *cons,
										  uint32_t offset1, uint32_t offset2) {
	return shell_col_num_with_buffer_offset_get(cons, offset2) -
		   shell_col_num_with_buffer_offset_get(cons, offset1);
}

int32_t
shell_row_span_with_buffer_offsets_get(struct shell_multiline_cons *cons,
									   uint32_t offset1, uint32_t offset2) {
	return shell_line_num_with_buffer_offset_get(cons, offset2) -
		   shell_line_num_with_buffer_offset_get(cons, offset1);
}

static bool shell_escape_sequence_handle(char **cmd) {
	char *current = *cmd;
	char next = *(current + 1);
	uint8_t value = 0;
	uint8_t i = 0;
	char digit = 0;

	if (next == '0') {
		for (i = 2; i < 5; i++) {
			digit = *(current + i);
			if (digit >= '0' && digit <= '7') {
				value = (value << 3) | (digit - '0');
			} else {
				break;
			}
		}

		if (i > 2) {
			memmove(current, current + (i - 1),
					shell_strlen(current) - (i - 2));
			*current = value;
			*cmd = current + 1;
			return true;
		}
	}

	if (next == 'x') {
		for (i = 2; i < 4; i++) {
			digit = *(current + i);
			if (digit >= '0' && digit <= '9') {
				value = (value << 4) | (digit - '0');
			} else if ((digit >= 'a') && (digit <= 'f')) {
				value = (value << 4) | (digit - 'a' + 10);
			} else if ((digit >= 'A') && (digit <= 'F')) {
				value = (value << 4) | (digit - 'A' + 10);
			} else {
				break;
			}
		}

		if (i > 2) {
			memmove(current, current + (i - 1),
					shell_strlen(current) - (i - 2));
			*current = value;
			*cmd = current + 1;
			return true;
		}
	}

	return false;
}

static char shell_internal_make_argv(char **ppcmd, uint8_t c) {
	char *cmd = *ppcmd;
	char quote = 0;
	char next = 0;

	while (1) {
		c = *cmd;

		if (c == '\0') {
			break;
		}

		if (!quote) {
			switch (c) {
			case '\\':
				memmove(cmd, cmd + 1, shell_strlen(cmd));
				cmd += 1;
				continue;

			case '\'':
			case '\"':
				memmove(cmd, cmd + 1, shell_strlen(cmd));
				quote = c;
				continue;
			default:
				break;
			}
		}

		if (quote == c) {
			memmove(cmd, cmd + 1, shell_strlen(cmd));
			quote = 0;
			continue;
		}

		if (quote && c == '\\') {
			next = *(cmd + 1);

			if (next == quote) {
				memmove(cmd, cmd + 1, shell_strlen(cmd));
				cmd += 1;
				continue;
			}

			if (shell_escape_sequence_handle(&cmd)) {
				continue;
			}
		}

		if (!quote && isspace((int)c) != 0) {
			break;
		}

		cmd += 1;
	}
	*ppcmd = cmd;

	return quote;
}

char shell_make_argv(int32_t *argc, char **argv, char *cmd, uint32_t max_argc) {
	char quote = 0;
	char c;

	*argc = 0;
	do {
		c = *cmd;
		if (c == '\0') {
			break;
		}

		if (isspace((int)c) != 0) {
			*cmd++ = '\0';
			continue;
		}

		argv[(*argc)++] = cmd;
		if (*argc == max_argc) {
			break;
		}
		quote = shell_internal_make_argv(&cmd, c);
	} while (true);

	return quote;
}

const struct shell_entry *
shell_get_last_command(const struct shell_entry *entry, int32_t argc,
					   char *argv[], int32_t *match_arg) {
	const struct shell_entry *prev_entry = NULL;
	*match_arg = SHELL_CMD_ROOT_LEVEL;

	while (*match_arg < argc) {
		prev_entry = entry;
		entry = shell_cmd_find(entry, argv[*match_arg]);
		if (entry) {
			(*match_arg)++;
		} else {
			entry = prev_entry;
			break;
		}
	}

	return entry;
}

size_t shell_strlen(const char *s) { return (s == NULL) ? 0 : strlen(s); }
