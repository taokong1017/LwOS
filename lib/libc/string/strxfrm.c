#include <string.h>

size_t strxfrm(char *dest, const char *src, size_t n)
{
	size_t l = strlen(src);
	if (n > l) strcpy(dest, src);
	return l;
}
