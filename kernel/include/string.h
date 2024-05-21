#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

/**
 * @brief Copies a specified number of characters from a source string to a
 * destination string
 * @param dest: Pointer to the destination string
 * @param src: Pointer to the source string
 * @param count: The maximum number of characters to copy
 * @return A pointer to the destination string
 */
char *strncpy(char *dest, const char *src, size_t count);

/**
 * @brief Calculate the length of a null-terminated string
 * @param s: Pointer to the null-terminated string
 * @return The length of the string (not including the null terminator)
 */
size_t strlen(const char *s);

/**
 * @brief Fill a block of memory with a specific value
 * @param dest: Pointer to the start of the memory block to fill
 * @param value: The value to fill the memory block with
 * @param numBytes: The number of bytes to fill
 */
void *memset(void *dest, unsigned int value, size_t numBytes);

#endif