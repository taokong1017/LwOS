#ifndef __CRYPT_H__
#define __CRYPT_H__

#ifdef __cplusplus
extern "C" {
#endif

struct crypt_data {
	int initialized;
	char __buf[256];
};

char *__crypt_r(const char *, const char *, struct crypt_data *);
char *__crypt_des(const char *, const char *, char *);
char *__crypt_md5(const char *, const char *, char *);
char *__crypt_blowfish(const char *, const char *, char *);
char *__crypt_sha256(const char *, const char *, char *);
char *__crypt_sha512(const char *, const char *, char *);

#ifdef __cplusplus
}
#endif

#endif
