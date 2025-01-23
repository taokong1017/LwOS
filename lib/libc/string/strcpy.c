#include <string.h>

char *strcpy(char *dest, const char *src)
{
	return stpcpy(dest, src);
}
