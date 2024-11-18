#ifndef __SHELL_UTIL_H__
#define __SHELL_UTIL_H__

#include <types.h>

void shell_transport_buffer_flush(struct shell *shell);

const struct shell_entry *shell_root_cmd_find(const char *syntax);
const struct shell_entry *shell_cmd_get(const struct shell_entry *parent,
										uint32_t index);
const struct shell_entry *shell_cmd_find(const struct shell_entry *parent,
										 const char *cmd_str);

void shell_spaces_trim(char *str);
void shell_pattern_remove(char *buff, size_t *buff_len, const char *pattern);
void shell_multiline_data_calc(struct shell_multiline_cons *cons,
							   uint16_t buff_pos, uint16_t buff_len);
#endif
