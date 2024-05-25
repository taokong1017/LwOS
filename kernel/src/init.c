#include <string.h>
#include <stdio.h>

#define SHELL_LOGO "[LS Kernel]# "
#define shell_logo_show() printf(SHELL_LOGO)
extern void logo_show();

void kernel_start() {
	logo_show();
	shell_logo_show();

	while (1) {
		/* TO DO */;
	}
}
