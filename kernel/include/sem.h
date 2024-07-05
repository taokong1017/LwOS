#ifndef __SEM_H__
#define __SEM_H__

#include <types.h>
#include <task.h>

/* sem name definition */
#define SEM_NAME_LEN 32

typedef long sem_id_t;

struct sem {
	sem_id_t id;
	char name[SEM_NAME_LEN];

	unsigned int count;
	unsigned int limit;

	wait_q_t wait_q;
};

#endif
