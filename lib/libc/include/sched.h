#ifndef __SCHED_H__
#define __SCHED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2
#define SCHED_BATCH 3
#define SCHED_IDLE 5
#define SCHED_DEADLINE 6

typedef long pid_t;
typedef int64_t time_t;
typedef struct cpu_set_t {
	unsigned long __bits[128 / sizeof(long)];
} cpu_set_t;

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};
struct sched_param {
	int sched_priority;
};

int sched_yield(void);
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);
int sched_getparam(pid_t, struct sched_param *);
int sched_getscheduler(pid_t);
int sched_rr_get_interval(pid_t, struct timespec *);
int sched_setparam(pid_t, const struct sched_param *);
int sched_setscheduler(pid_t, int, const struct sched_param *);
int __sched_cpucount(size_t, const cpu_set_t *);
int sched_getcpu(void);
int sched_getaffinity(pid_t, size_t, cpu_set_t *);
int sched_setaffinity(pid_t, size_t, const cpu_set_t *);

#ifdef __cplusplus
}
#endif

#endif
