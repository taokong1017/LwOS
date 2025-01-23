#include <string.h>

char *strncpy(char *d, const char *s, size_t n)
{
	return stpncpy(d, s, n);
}
