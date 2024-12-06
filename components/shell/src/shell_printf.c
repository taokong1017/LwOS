#include <shell_printf.h>
#include <stdio.h>

int32_t shell_printf(struct shell_printf *shell_printf, const char *format,
					 va_list args) {
	int32_t ret = 0;

	if ((!shell_printf) || (!shell_printf->buffer)) {
		return 0;
	}

	ret = vsnprintf(shell_printf->buffer, shell_printf->buffer_size, format,
					args);
	shell_printf->control->buffer_count = ret;

	if (shell_printf->control->flush) {
		shell_printf_flush(shell_printf);
	}

	return ret;
}

void shell_printf_flush(struct shell_printf *shell_printf) {
	if ((!shell_printf) || (!shell_printf->buffer) || (!shell_printf->write)) {
		return;
	}

	shell_printf->write(shell_printf->context, shell_printf->buffer,
						shell_printf->control->buffer_count);
	shell_printf->control->buffer_count = 0;

	return;
}
