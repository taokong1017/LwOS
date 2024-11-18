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
		if ((strlen(syntax) == strlen(cmd_entry->entry->syntax)) &&
			(strcmp(syntax, cmd_entry->entry->syntax) == 0)) {
			return cmd_entry->entry;
		}
	}

	return NULL;
}

static bool is_section_subcmd(const struct shell_cmd_entry *cmd_entry) {
	type_section_start_extern(struct shell_cmd_entry, shell_sub_cmd);
	type_section_end_extern(struct shell_cmd_entry, shell_sub_cmd);

	return (cmd_entry >= type_section_start(shell_sub_cmd)) &&
		   (cmd_entry < type_section_end(shell_sub_cmd));
}

const struct shell_entry *shell_cmd_get(const struct shell_entry *parent,
										uint32_t index) {
	const struct shell_entry *entry = NULL;
	const struct shell_cmd_entry *entry_list = NULL;

	if (parent == NULL) {
		return (index < shell_root_cmd_count())
				   ? shell_root_cmd_get(index)->entry
				   : NULL;
	}

	if (is_section_subcmd(parent->subcmd)) {
		entry_list = (const struct shell_cmd_entry *)parent->subcmd;
		if (entry_list[index].entry->syntax != NULL) {
			entry = entry_list[index].entry;
		}
	}

	return entry;
}

const struct shell_entry *shell_cmd_find(const struct shell_entry *parent,
										 const char *cmd_str) {
	const struct shell_entry *entry;
	size_t index = 0;

	while ((entry = shell_cmd_get(parent, index++)) != NULL) {
		if ((strlen(cmd_str) == strlen(entry->syntax)) &&
			(strcmp(cmd_str, entry->syntax) == 0)) {
			return entry;
		}
	}

	return NULL;
}

void shell_spaces_trim(char *str) {
	int32_t len = strlen(str);
	int32_t shift = 0U;
	int32_t i = 0;
	int32_t j = 0;

	if (!str || (len == 0U)) {
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
					shift = 0U;
				}

				break;
			}
		}
	}
}

void shell_pattern_remove(char *buff, size_t *buff_len, const char *pattern) {
	char *pattern_addr = strstr(buff, pattern);
	uint32_t pattern_len = strlen(pattern);
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

	shift = strlen(pattern_addr) - pattern_len + 1; /* +1 for EOS */
	*buff_len -= pattern_len;

	memmove(pattern_addr, pattern_addr + pattern_len, shift);
}

void shell_multiline_data_calc(struct shell_multiline_cons *cons,
							   uint16_t buff_pos, uint16_t buff_len) {
	cons->cur_x = (buff_pos + cons->name_len) % cons->terminal_width + 1;
	cons->cur_y = (buff_pos + cons->name_len) / cons->terminal_width + 1;
	cons->cur_y_end = (buff_len + cons->name_len) / cons->terminal_width + 1;
	cons->cur_x_end = (buff_len + cons->name_len) % cons->terminal_width + 1;
}
