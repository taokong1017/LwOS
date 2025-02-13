#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#define ULONG_MAX U32_MAX

unsigned long strtoul(const char *nptr, char **endptr, register int base)
{
	register const char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while (isspace((unsigned char)c) != 0);
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+') {
		c = *s++;
	}

	if (((base == 0) || (base == 16)) &&
	    (c == '0') && ((*s == 'x') || (*s == 'X'))) {
		c = s[1];
		s += 2;
		base = 16;
	}

	if (base == 0) {
		base = (c == '0') ? 8 : 10;
	}

	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit((unsigned char)c) != 0) {
			c -= '0';
		} else if (isalpha((unsigned char)c) != 0) {
			c -= isupper((unsigned char)c) != 0 ? 'A' - 10 : 'a' - 10;
		} else {
			break;
		}
		if (c >= base) {
			break;
		}
		if ((any < 0) || (acc > cutoff) || ((acc == cutoff) && (c > cutlim))) {
			any = -1;
		} else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
	} else if (neg != 0) {
		acc = -acc;
	}
	if (endptr != NULL) {
		*endptr = (char *)(any ? (s - 1) : nptr);
	}

	return acc;
}
