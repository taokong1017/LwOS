
#include <log.h>
#include <stdio.h>
#include <arch_spinlock.h>

#define BUFFER_SIZE 256

extern int32_t uart_puts(const char *str, int32_t len);
static const char *level_strings[] = {"FATAL", "ERROR", "INFO", "DEBUG"};
static output_func log_output_func = uart_puts;
static int32_t log_level = LOG_LEVEL_DEBUG;

void log_init(int32_t level, output_func output) {
	if (LOG_LEVEL_IS_INVALID(level)) {
		log_level = level;
	}

	log_output_func = output;
}

static int32_t tag_output(const char *fmt, ...) {
	va_list args;
	int32_t ret;
	char buffer[BUFFER_SIZE] = {0};

	va_start(args, fmt);
	ret = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, args);
	va_end(args);

	if (log_output_func) {
		log_output_func(buffer, ret);
	}

	return ret;
}

int32_t log_output(int32_t level, const char *tag, const char *format, ...) {
	va_list args;
	int32_t ret = 0;
	char buffer[BUFFER_SIZE] = {0};

	if (LOG_LEVEL_IS_INVALID(level) || !tag || !format) {
		return -1;
	}

	if (level > log_level) {
		return -1;
	}

	tag_output("[%s]<%s>: ", level_strings[level], tag);

	va_start(args, format);
	ret = vsnprintf(buffer, BUFFER_SIZE - 1, format, args);
	va_end(args);

	if (log_output_func) {
		log_output_func(buffer, ret);
	}

	return ret;
}
