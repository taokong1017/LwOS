#ifndef __SHELL_PRINTF_H__
#define __SHELL_PRINTF_H__

#include <types.h>
#include <stdarg.h>

typedef void (*shell_write_t)(void *user_context, char *data, size_t length);

struct shell_printf_control {
	size_t buffer_count;
	bool auto_flush;
};

struct shell_printf {
	char *buffer;
	size_t buffer_size;
	const shell_write_t write;
	void *user_context;
	struct shell_printf_control *control;
};

#define shell_buffer_define(_name, _buffer, _size, _write, _user_context,      \
							_auto_flush)                                       \
	static struct shell_printf_control _name##_shell_printf_control = {        \
		.buffer_count = 0U,                                                    \
		.auto_flush = _auto_flush,                                             \
	};                                                                         \
	static struct shell_printf _name##_shell_printf = {                        \
		.buffer = _buffer,                                                     \
		.buffer_size = _size,                                                  \
		.write = _write,                                                       \
		.user_context = _user_context,                                         \
		.control = &_name##_shell_printf_control,                              \
	};

/* Shell printf interface */
int32_t shell_printf(struct shell_printf *shell_printf, const char *format,
					 va_list args);
void shell_printf_flush(struct shell_printf *shell_printf);

#endif
