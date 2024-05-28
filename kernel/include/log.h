#ifndef __LOG_H__
#define __LOG_H__

#include <types.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LOWEST_LEVEL LOG_LEVEL_DEBUG
#define LOG_LEVEL_IS_INVALID(level)                                            \
	((level < LOG_LEVEL_ERROR || level > LOG_LEVEL_DEBUG) ? true : false)
static const char *level_strings[] = {"ERROR", "INFO", "DEBUG"};

static int log_output(int level, const char *tag, const char *format, ...) {
	va_list args;
	int32_t ret = 0;

	if (LOG_LEVEL_IS_INVALID(level) || !tag || !format) {
		return -1;
	}

	if (level > LOG_LOWEST_LEVEL) {
		return -1;
	}

	ret = printf("[%s]<%s>: ", level_strings[level], tag);
	va_start(args, format);
	ret += printf(format, args);
	va_end(args);

	return ret;
}

#define log_err(tag, format, ...)                                              \
	log_output(LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)
#define log_info(tag, format, ...)                                             \
	log_output(LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#define log_debug(tag, format, ...)                                            \
	log_output(LOG_LEVEL_DEBUG, tag, format, ##__VA_ARGS__)

#endif
