#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *);
long atol(const char *);
long long atoll(const char *);
double atof(const char *);

float strtof(const char *, char **);
double strtod(const char *, char **);
long double strtold(const char *, char **);

long strtol(const char *, char **, int);
unsigned long strtoul(const char *, char **, int);
long long strtoll(const char *, char **, int);
unsigned long long strtoull(const char *, char **, int);

int rand(void);
void srand(unsigned);

void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
void *aligned_alloc(size_t, size_t);

void *bsearch(const void *, const void *, size_t, size_t, int(*)(const void *, const void *));
void qsort(void *, size_t, size_t, int(*)(const void *, const void *));

int abs(int);
long labs(long);
long long llabs(long long);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div(int, int);
ldiv_t ldiv(long, long);
lldiv_t lldiv(long long, long long);

int mblen(const char *, size_t);
int mbtowc(wchar_t *, const char *, size_t);
int wctomb(char *, wchar_t);
size_t mbstowcs(wchar_t *, const char *, size_t);
size_t wcstombs(char *, const wchar_t *, size_t);

#ifdef __cplusplus
}
#endif

#endif
