#include <string.h>

char *strcat(char *dest, const char *src) {
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

char *strncat(char *dest, const char *src, size_t count) {
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

size_t strlen(const char *s) {
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
int strcmp(const char *cs, const char *ct) {
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int strncmp(const char *cs, const char *ct, size_t count) {
	unsigned char c1, c2;

	while (count) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}

size_t strnlen(const char *s, size_t count) {
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/**
 * memset64() - Fill a memory area with a uint64_t
 * @s: Pointer to the start of the area.
 * @v: The value to fill the area with
 * @count: The number of values to store
 *
 * Differs from memset() in that it fills with a uint64_t instead
 * of a byte.  Remember that @count is the number of uint64_ts to
 * store, not the number of bytes.
 */
void *memset64(uint64_t *s, uint64_t v, size_t count) {
	uint64_t *xs = s;

	while (count--)
		*xs++ = v;
	return s;
}

/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
char *strstr(const char *s1, const char *s2) {
	size_t l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}
	return NULL;
}

/**
 * @brief Searches for the first occurrence of a character in a string
 * @param s: Pointer to the string to be searched
 * @param c: The character to search for
 * @return A pointer to the first occurrence of the character, or NULL if it's
 * not found
 */
char *strchr(const char *s, int c) {
	char tmp = (char)c;

	while ((*s != tmp) && (*s != '\0')) {
		s++;
	}

	return (*s == tmp) ? (char *)s : NULL;
}
