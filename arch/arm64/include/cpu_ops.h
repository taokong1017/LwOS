#ifndef __CPU_OPS_H__
#define __CPU_OPS_H__

#include <types.h>
#include <kerrno.h>

enum cpu_reset_type { CPU_WARM_RESET, CPU_COLD_RESET };

void sys_poweroff();
errno_t sys_reset(enum cpu_reset_type type);
errno_t cpu_on(uint64_t cpuid, uintptr_t entry_point);
errno_t cpu_off();

#endif
