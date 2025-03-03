#ifndef __SHELL_UTIL_H__
#define __SHELL_UTIL_H__

#include <types.h>

void shell_transport_buffer_flush(struct shell *shell);

const struct shell_entry *shell_root_cmd_find(const char *syntax);
const struct shell_entry *shell_cmd_get(const struct shell_entry *parent,
										uint32_t index);
const struct shell_entry *shell_cmd_find(const struct shell_entry *parent,
										 const char *cmd_str);
void shell_cmd_space_trim(struct shell *shell);

void shell_spaces_trim(char *str);
void shell_pattern_remove(char *buff, size_t *buff_len, const char *pattern);
void shell_multiline_data_calc(struct shell_multiline_cons *cons,
							   uint32_t buff_pos, uint32_t buff_len);

int32_t
shell_column_span_with_buffer_offsets_get(struct shell_multiline_cons *cons,
										  uint32_t offset1, uint32_t offset2);
int32_t
shell_row_span_with_buffer_offsets_get(struct shell_multiline_cons *cons,
									   uint32_t offset1, uint32_t offset2);

char shell_make_argv(int32_t *argc, char **argv, char *cmd, uint32_t max_argc);
const struct shell_entry *
shell_get_last_command(const struct shell_entry *entry, int32_t argc,
					   char *argv[], int32_t *match_arg);

size_t shell_strlen(const char *s);

#endif
