#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

/**
 * @brief Copies characters from a source string to a destination string
 * @param dest: Pointer to the destination string
 * @param src: Pointer to the source string
 * @return A pointer to the destination string
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief Copies a specified number of bytes from a source memory area to a
 * destination memory area
 * @param dest: Pointer to the destination memory area
 * @param src: Pointer to the source memory area
 * @param count: The maximum number of bytes to copy
 * @return A pointer to the destination memory area
 */
void *memmove(void *dest, const void *src, size_t count);

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
 * @brief Copies a specified number of characters from a source string to a
 * destination string
 * @param dest: Pointer to the destination string
 * @param src: Pointer to the source string
 * @param count: The maximum number of characters to copy
 * @return A pointer to the destination string
 */
char *strncpy(char *dest, const char *src, size_t count);

/**
 * @brief Copies a specified number of bytes from a source memory area to a
 * destination memory area
 * @param dest: Pointer to the destination memory area
 * @param src: Pointer to the source memory area
 * @param count: The maximum number of bytes to copy
 * @return A pointer to the destination memory area
 */
void *memmove(void *dest, const void *src, size_t count);

/**
 * @brief Appends a specified number of characters from a source string to a
 * destination string
 * @param dest: Pointer to the destination string
 * @param src: Pointer to the source string
 * @param count: The maximum number of characters to append
 * @return A pointer to the destination string
 */
char *strncat(char *dest, const char *src, size_t count);

/**
 * @brief Compares two strings
 * @param cs: Pointer to the first string
 * @param ct: Pointer to the second string
 * @return A negative value if cs is lexicographically smaller than ct,
 * 0 if they are equal, a positive value if cs is lexicographically greater
 * than ct
 */
int strcmp(const char *cs, const char *ct);

/**
 * @brief Compares a specified number of characters from two strings
 * @param cs: Pointer to the first string
 * @param ct: Pointer to the second string
 * @param count: The number of characters to compare
 * @return A negative value if cs is lexicographically smaller than ct,
 * 0 if they are equal, a positive value if cs is lexicographically greater
 * than ct
 */
int strncmp(const char *cs, const char *ct, size_t count);

/**
 * @brief Returns the length of a string
 * @param s: Pointer to the string
 * @return The length of the string
 */
size_t strlen(const char *s);

/**
 * @brief Fills a memory area with a specific value
 * @param s: Pointer to the memory area
 * @param c: The value to fill the memory area with
 * @param count: The number of bytes to fill
 * @return A pointer to the memory area
 */
void *memset(void *s, int c, size_t count);

/**
 * @brief Fills a memory area with a specific 64-bit value
 * @param s: Pointer to the memory area
 * @param v: The 64-bit value to fill the memory area with
 * @param count: The number of bytes to fill
 * @return A pointer to the memory area
 */
void *memset64(uint64_t *s, uint64_t v, size_t count);

/**
 * @brief Compares two memory areas
 * @param cs: Pointer to the first memory area
 * @param ct: Pointer to the second memory area
 * @param count: The number of bytes to compare
 * @return A negative value if cs is lexicographically smaller than ct,
 * 0 if they are equal, a positive value if cs is lexicographically
 * greater than ct
 */
int memcmp(const void *cs, const void *ct, size_t count);

/**
 * @brief Searches for the first occurrence of a substring in a string
 * @param s1: Pointer to the string to be searched
 * @param s2: Pointer to the substring to search for
 * @return A pointer to the first occurrence of the substring, or NULL
 * if it's not found
 */
char *strstr(const char *s1, const char *s2);

/**
 * @brief Searches for the first occurrence of a character in a string
 * @param s: Pointer to the string to be searched
 * @param c: The character to search for
 * @return A pointer to the first occurrence of the character, or NULL if it's
 * not found
 */
char *strchr(const char *s, int c);

#endif