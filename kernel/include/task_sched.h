#ifndef __TASK_SCHED_H__
#define __TASK_SCHED_H__

#include <kernel.h>

#define forever() for (;;)

void task_announce();
void task_sched_init();

#endif
