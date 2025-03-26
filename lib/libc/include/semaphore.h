#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sem.h>

typedef struct sem sem_t;

int sem_close(sem_t *);
int sem_destroy(sem_t *);
int sem_getvalue(sem_t *, int *);
int sem_init(sem_t *, int, unsigned);
sem_t *sem_open(const char *, int, ...);
int sem_post(sem_t *);
int sem_timedwait(sem_t *, const struct timespec *);
int sem_trywait(sem_t *);
int sem_unlink(const char *);
int sem_wait(sem_t *);

#ifdef __cplusplus
}
#endif

#endif
