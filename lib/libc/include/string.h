#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
int strcoll(const char *, const char *);
size_t strxfrm(char *, const char *, size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
size_t strcspn(const char *, const char *);
size_t strspn(const char *, const char *);
char *strpbrk(const char *, const char *);
char *strstr(const char *, const char *);
size_t strlen(const char *);
char *strtok_r(char *, const char *, char **);
char *stpcpy(char *, const char *);
char *stpncpy(char *, const char *, size_t);
size_t strnlen(const char *, size_t);
char *strdup(const char *);
char *strndup(const char *, size_t);
void *memmem(const void *, size_t, const void *, size_t);
void *memccpy(void *, const void *, int, size_t);
char *strsep(char **, const char *);
size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
void explicit_bzero(void *, size_t);
int strverscmp(const char *, const char *);
char *strchrnul(const char *, int);
char *strcasestr(const char *, const char *);
void *memrchr(const void *, int, size_t);
void *mempcpy(void *, const void *, size_t);

#ifdef __cplusplus
}
#endif

#endif
