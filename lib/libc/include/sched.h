#ifndef __SCHED_H__
#define __SCHED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define SCHED_RR 1

int sched_yield(void);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);

#ifdef __cplusplus
}
#endif

#endif
