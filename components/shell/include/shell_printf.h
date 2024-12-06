#ifndef __SHELL_PRINTF_H__
#define __SHELL_PRINTF_H__

#include <types.h>
#include <stdarg.h>

typedef void (*shell_write_func)(void *context, char *data, uint32_t length);

struct shell_printf_control {
	size_t buffer_count;
	bool flush;
};

struct shell_printf {
	char *buffer;
	size_t buffer_size;
	shell_write_func write;
	void *context;
	struct shell_printf_control *control;
};

#define shell_printf_define(name, printf_buffer, printf_buffer_size,           \
							write_func, user_context, auto_flush)              \
	static struct shell_printf_control name##_shell_printf_control = {         \
		.buffer_count = 0,                                                     \
		.flush = auto_flush,                                                   \
	};                                                                         \
	static struct shell_printf name##_shell_printf = {                         \
		.buffer = printf_buffer,                                               \
		.buffer_size = printf_buffer_size,                                     \
		.write = write_func,                                                   \
		.context = user_context,                                               \
		.control = &name##_shell_printf_control,                               \
	};

/* Shell printf interface */
int32_t shell_printf(struct shell_printf *shell_printf, const char *format,
					 va_list args);
void shell_printf_flush(struct shell_printf *shell_printf);

#endif
