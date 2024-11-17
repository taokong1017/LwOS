#ifndef __SHELL_WILDCARD_H__
#define __SHELL_WILDCARD_H__

#include <types.h>

enum shell_wildcard_status {
	SHELL_WILDCARD_ADDED,
	SHELL_WILDCARD_MISSING_SPACE,
	SHELL_WILDCARD_NO_MATCH_FOUND,
	SHELL_WILDCARD_NOT_FOUND,
};

bool shell_has_wildcard(const char *str);
void shell_wildcard_prepare(struct shell *shell);
enum shell_wildcard_status shell_wildcard_process(struct shell *shell,
												  const struct shell_entry *cmd,
												  const char *pattern);
void shell_wildcard_finalize(struct shell *shell);

#endif
