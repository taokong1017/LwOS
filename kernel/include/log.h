#ifndef __LOG_H__
#define __LOG_H__

#include <types.h>
#include <stdio.h>
#include <stdarg.h>

#define BUFFER_SIZE 256
#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3
#define LOG_LOWEST_LEVEL LOG_LEVEL_DEBUG
#define LOG_LEVEL_IS_INVALID(level)                                            \
	((level < LOG_LEVEL_FATAL || level > LOG_LEVEL_DEBUG) ? true : false)

typedef int32_t (*output_func)(const char *str, int32_t len);
extern int32_t uart_puts(const char *str, int32_t len);
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
static const char *level_strings[] = {"FATAL", "ERROR", "INFO", "DEBUG"};
static output_func log_output_func = uart_puts;

static int tag_output(const char *fmt, ...) {
	va_list args;
	int ret;
	char buffer[BUFFER_SIZE] = {0};

	va_start(args, fmt);
	ret = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, args);
	va_end(args);
	if (log_output_func) {
		log_output_func(buffer, ret);
	}

	return ret;
}

static int log_output(int level, const char *tag, const char *format, ...) {
	va_list args;
	int32_t ret = 0;
	char buffer[BUFFER_SIZE] = {0};

	if (LOG_LEVEL_IS_INVALID(level) || !tag || !format) {
		return -1;
	}

	if (level > LOG_LOWEST_LEVEL) {
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

#define log_fatal(tag, format, args...)                                        \
	do {                                                                       \
		log_output(LOG_LEVEL_FATAL, tag, format, ##args);                      \
		forever();                                                             \
	} while (0)
#define log_err(tag, format, args...)                                          \
	do {                                                                       \
		log_output(LOG_LEVEL_ERROR, tag, format, ##args);                      \
	} while (0)
#define log_info(tag, format, args...)                                         \
	do {                                                                       \
		log_output(LOG_LEVEL_INFO, tag, format, ##args);                       \
	} while (0)
#define log_debug(tag, format, args...)                                        \
	do {                                                                       \
		log_output(LOG_LEVEL_DEBUG, tag, format, ##args);                      \
	} while (0)

#endif
