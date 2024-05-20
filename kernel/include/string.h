#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

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