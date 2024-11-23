#include <shell.h>
#include <string.h>
#include <fnmatch.h>

bool shell_has_wildcard(const char *str) {
	uint16_t len = strlen(str);
	int32_t index = 0;

	for (index = 0; index < len; index++) {
		if ((str[index] == '?') || (str[index] == '*')) {
			return true;
		}
	}

	return false;
}

void shell_wildcard_prepare(struct shell *shell) {
	memset(shell->shell_context->temp_buffer, 0,
		   sizeof(shell->shell_context->temp_buffer));
	memcpy(shell->shell_context->temp_buffer, shell->shell_context->cmd_buffer,
		   shell->shell_context->cmd_buffer_length);
	shell_spaces_trim(shell->shell_context->temp_buffer);
	shell->shell_context->temp_buffer_len =
		strlen(shell->shell_context->temp_buffer) + 1; /* +1 for EOS */
}

static enum shell_wildcard_status shell_command_add(char *buff,
													size_t *buff_len,
													char const *cmd,
													char const *pattern) {
	uint32_t cmd_len = strlen(cmd);
	char *completion_addr;
	uint32_t shift;

	if ((*buff_len + cmd_len + 1) > CONFIG_SHELL_CMD_BUFFER_SIZE) {
		return SHELL_WILDCARD_MISSING_SPACE;
	}

	completion_addr = strstr(buff, pattern);

	if (!completion_addr) {
		return SHELL_WILDCARD_NO_MATCH_FOUND;
	}

	shift = strlen(completion_addr);

	memmove(completion_addr + cmd_len + 1, completion_addr, shift + 1);
	memcpy(completion_addr, cmd, cmd_len);
	completion_addr[cmd_len] = ' ';
	*buff_len += cmd_len + 1;

	return SHELL_WILDCARD_ADDED;
}

static enum shell_wildcard_status
shell_commands_expand(struct shell *shell, const struct shell_entry *cmd,
					  const char *pattern) {
	enum shell_wildcard_status ret_val = SHELL_WILDCARD_NO_MATCH_FOUND;
	const struct shell_entry *entry = NULL;
	int32_t index = 0;
	int32_t count = 0;

	while ((entry = shell_cmd_get(cmd, index++)) != NULL) {

		if (fnmatch(pattern, entry->syntax, 0) == 0) {
			ret_val = shell_command_add(shell->shell_context->temp_buffer,
										&shell->shell_context->temp_buffer_len,
										entry->syntax, pattern);
			if (ret_val == SHELL_WILDCARD_MISSING_SPACE) {
				shell_color_show(
					shell, SHELL_WARNING,
					"Command buffer is too short to expand all commands matching wildcard pattern: %s\n",
					pattern);
				break;
			} else if (ret_val != SHELL_WILDCARD_ADDED) {
				break;
			}
			count++;
		}
	}

	if (count > 0) {
		shell_pattern_remove(shell->shell_context->temp_buffer,
							 &shell->shell_context->temp_buffer_len, pattern);
	}

	return ret_val;
}

enum shell_wildcard_status shell_wildcard_process(struct shell *shell,
												  const struct shell_entry *cmd,
												  const char *pattern) {
	if (cmd == NULL) {
		return SHELL_WILDCARD_NOT_FOUND;
	}

	if (!shell_has_wildcard(pattern)) {
		return SHELL_WILDCARD_NOT_FOUND;
	}

	return shell_commands_expand(shell, cmd, pattern);
}

void shell_wildcard_finalize(struct shell *shell) {
	memcpy(shell->shell_context->cmd_buffer, shell->shell_context->temp_buffer,
		   shell->shell_context->temp_buffer_len);
	shell->shell_context->cmd_buffer_length =
		shell->shell_context->temp_buffer_len;
}
