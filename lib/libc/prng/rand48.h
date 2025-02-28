#ifndef __RAND48_H__
#define __RAND48_H__

#include <types.h>

extern uint64_t __rand48_step(unsigned short *xi, unsigned short *lc);
extern unsigned short __seed48[7];

#endif
