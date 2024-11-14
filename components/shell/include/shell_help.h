#ifndef __SHELL_HELP_H__
#define __SHELL_HELP_H__

#include <shell.h>

void shell_help_cmd_print(struct shell *shell, const struct shell_entry *entry);
void shell_help_subcmd_print(struct shell *shell,
							 const struct shell_entry *entry,
							 const char *description);
bool shell_help_cmd_is_request(const char *str);

#endif
