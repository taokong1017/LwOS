#ifndef __CRYPT_DES_H__
#define __CRYPT_DES_H__

#include <types.h>

struct expanded_key {
	uint32_t l[16], r[16];
};

void __des_setkey(const unsigned char *, struct expanded_key *);
void __do_des(uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t, uint32_t,
			  const struct expanded_key *);

#endif
