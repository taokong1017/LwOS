#ifndef __SHELL_PRINTF_H__
#define __SHELL_PRINTF_H__

#include <types.h>

typedef void (*shell_write_t)(void *user_context, uint8_t *data, size_t length);

struct shell_printf {
	uint8_t *buffer;
	size_t buffer_size;
	shell_write_t write;
	void *user_context;
};

#endif
