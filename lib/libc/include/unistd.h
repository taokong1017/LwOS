#ifndef __UNISTD_H__
#define __UNISTD_H__

#ifdef __cplusplus
extern "C" {
#endif

char *crypt(const char *, const char *);
void encrypt(char *, int);

#ifdef __cplusplus
}
#endif

#endif
