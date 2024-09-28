#ifndef __LOG_H__
#define __LOG_H__

#include <types.h>
#include <stdio.h>
#include <general.h>

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_IS_INVALID(level)                                            \
	((level < LOG_LEVEL_FATAL || level > LOG_LEVEL_DEBUG) ? true : false)

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

typedef int32_t (*output_func)(const char *str, int32_t len);

int32_t log_output(int32_t level, const char *tag, const char *format, ...);
void log_init(int32_t level, output_func output);
bool log_level_set(int32_t level);
int32_t log_level_get();

#endif
