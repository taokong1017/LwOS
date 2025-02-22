#include <shell_printf.h>
#include <stdio.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int32_t shell_raw_printf(struct shell_printf *shell_printf, const char *data,
						 size_t len) {
	size_t min_len = 0;

	if ((!shell_printf) || (!shell_printf->buffer)) {
		return 0;
	}

	min_len = min(shell_printf->buffer_size, len);
	memcpy(shell_printf->buffer, data, min_len);
	shell_printf->control->buffer_count = min_len;

	if (shell_printf->control->flush) {
		shell_printf_flush(shell_printf);
	}

	return min_len;
}

int32_t shell_printf(struct shell_printf *shell_printf, const char *format,
					 va_list args) {
	int32_t ret = 0;

	if ((!shell_printf) || (!shell_printf->buffer)) {
		return 0;
	}

	ret = vsnprintf(shell_printf->buffer, shell_printf->buffer_size, format,
					args);
	shell_printf->control->buffer_count = min(ret, shell_printf->buffer_size);

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
