#ifndef __SMP_H__
#define __SMP_H__

#include <types.h>

void smp_init();
void smp_cpu_start(uint32_t cpu_id);

#endif
