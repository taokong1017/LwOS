#ifndef __SHELL_UTIL_H__
#define __SHELL_UTIL_H__

#include <types.h>
#include <shell.h>

void shell_transport_buffer_flush(struct shell *shell);

const struct shell_entry *shell_root_cmd_find(const char *syntax);
const struct shell_entry *shell_cmd_get(const struct shell_entry *parent,
										uint32_t index);
const struct shell_entry *shell_cmd_find(const struct shell_entry *parent,
										 const char *cmd_str);

#endif
