#include <string.h>
#include <limits.h>

#define UCHAR_MAX U8_MAX
#define ALIGN (sizeof(size_t)-1)
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) (((x)-ONES) & ~(x) & HIGHS)

char *stpncpy(char *d, const char *s, size_t n)
{
	typedef size_t __attribute__((__may_alias__)) word;
	word *wd;
	const word *ws;

	if (((uintptr_t)s & ALIGN) == ((uintptr_t)d & ALIGN)) {
		for (; ((uintptr_t)s & ALIGN) && n && (*d=*s); n--, s++, d++);
		if (!n || !*s) goto tail;
		wd=(void *)d; ws=(const void *)s;
		for (; n>=sizeof(size_t) && !HASZERO(*ws);
		       n-=sizeof(size_t), ws++, wd++) *wd = *ws;
		d=(void *)wd; s=(const void *)ws;
	}

	for (; n && (*d=*s); n--, s++, d++);

tail:
	memset(d, 0, n);

	return d;
}
