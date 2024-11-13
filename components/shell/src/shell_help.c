#include <string.h>
#include <shell_help.h>

bool shell_help_cmd_request(const char *str) {
	if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
		return true;
	}

	return false;
}
