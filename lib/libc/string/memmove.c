#include <string.h>

typedef __attribute__((__may_alias__)) size_t WT;
#define WS (sizeof(WT))

void *memmove(void *dest, const void *src, size_t n) {
	char *d = dest;
	const char *s = src;

	if (d == s)
		return d;
	if ((uintptr_t)s - (uintptr_t)d - n <= ~((uintptr_t)(2 * n) - 1))
		return memcpy(d, s, n);

	if (d < s) {
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)d % WS) {
				if (!n--)
					return dest;
				*d++ = *s++;
			}
			for (; n >= WS; n -= WS, d += WS, s += WS)
				*(WT *)d = *(WT *)s;
		}
		for (; n; n--)
			*d++ = *s++;
	} else {
		if ((uintptr_t)s % WS == (uintptr_t)d % WS) {
			while ((uintptr_t)(d + n) % WS) {
				if (!n--)
					return dest;
				d[n] = s[n];
			}
			while (n >= WS)
				n -= WS, *(WT *)(d + n) = *(WT *)(s + n);
		}
		while (n)
			n--, d[n] = s[n];
	}

	return dest;
}
