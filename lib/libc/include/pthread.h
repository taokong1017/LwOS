#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sched.h>
#include <mutex.h>

#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1
#define PTHREAD_CANCEL_MASKED 2

#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1
#define PTHREAD_CANCELED ((void *)-1)

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2

#define PTHREAD_MUTEX_STALLED 0
#define PTHREAD_MUTEX_ROBUST 1

#define PTHREAD_ONCE_INIT 0

#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1

#define PTHREAD_MUTEX_INITIALIZER                                              \
	{ 0 }
#define PTHREAD_RWLOCK_INITIALIZER                                             \
	{ 0 }
#define PTHREAD_COND_INITIALIZER                                               \
	{ 0 }

typedef long pthread_t;
typedef long pthread_condattr_t;
typedef long pthread_rwlockattr_t;
typedef long pthread_mutexattr_t;
typedef long pthread_barrierattr_t;
typedef int pthread_key_t;
typedef int pthread_once_t;
typedef struct pthread_attr pthread_attr_t;

struct pthread_attr {
	void *stackaddr;
	int stacksize;

	int inheritsched;
	int schedpolicy;
	struct sched_param schedparam;

	int detachstate;
};

struct pthread_mutex {
	pthread_mutexattr_t attr;
	struct mutex lock;
};
typedef struct pthread_mutex pthread_mutex_t;

struct pthread_cond {
	pthread_condattr_t attr;
	struct ksem sem;
};
typedef struct pthread_cond pthread_cond_t;

struct pthread_rwlock {
	pthread_rwlockattr_t attr;
	pthread_mutex_t rw_mutex;
	pthread_cond_t rw_condreaders;
	pthread_cond_t rw_condwriters;
	int rw_nwaitreaders;
	int rw_nwaitwriters;
	int rw_refcount;
};
typedef struct pthread_rwlock pthread_rwlock_t;

struct pthread_barrier {
	int count;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
};
typedef struct pthread_barrier pthread_barrier_t;

int pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *),
				   void *);
int pthread_once(pthread_once_t *, void (*)(void));
int pthread_join(pthread_t, void **);
int pthread_detach(pthread_t);
int pthread_equal(pthread_t, pthread_t);
void pthread_exit(void *);
pthread_t pthread_self(void);

int pthread_attr_init(pthread_attr_t *);
int pthread_attr_destroy(pthread_attr_t *);
int pthread_attr_getstack(const pthread_attr_t *, void **, size_t *);
int pthread_attr_setstack(pthread_attr_t *, void *, size_t);
int pthread_attr_getstacksize(const pthread_attr_t *, size_t *);
int pthread_attr_setstacksize(pthread_attr_t *, size_t);
int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
int pthread_attr_setdetachstate(pthread_attr_t *, int);
int pthread_attr_getguardsize(const pthread_attr_t *, size_t *);
int pthread_attr_setguardsize(pthread_attr_t *, size_t);
int pthread_attr_getschedparam(const pthread_attr_t *, struct sched_param *);
int pthread_attr_setschedparam(pthread_attr_t *, const struct sched_param *);
int pthread_attr_getschedpolicy(const pthread_attr_t *, int *);
int pthread_attr_setschedpolicy(pthread_attr_t *, int);
int pthread_attr_getinheritsched(const pthread_attr_t *, int *);
int pthread_attr_setinheritsched(pthread_attr_t *, int);
int pthread_attr_getscope(const pthread_attr_t *, int *);
int pthread_attr_setscope(pthread_attr_t *, int);

int pthread_setcancelstate(int, int *);
int pthread_setcanceltype(int, int *);
void pthread_testcancel(void);
int pthread_cancel(pthread_t);

int pthread_key_create(pthread_key_t *, void (*)(void *));
int pthread_key_delete(pthread_key_t);
int pthread_setspecific(pthread_key_t, const void *);
void *pthread_getspecific(pthread_key_t);

int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_destroy(pthread_mutex_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_trylock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);

int pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int pthread_cond_destroy(pthread_cond_t *);
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
						   const struct timespec *);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_broadcast(pthread_cond_t *);

int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
int pthread_rwlock_destroy(pthread_rwlock_t *);
int pthread_rwlock_rdlock(pthread_rwlock_t *);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int pthread_rwlock_wrlock(pthread_rwlock_t *);
int pthread_rwlock_trywrlock(pthread_rwlock_t *);
int pthread_rwlock_unlock(pthread_rwlock_t *);

int pthread_barrier_init(pthread_barrier_t *, const pthread_barrierattr_t *,
						 unsigned);
int pthread_barrier_destroy(pthread_barrier_t *);
int pthread_barrier_wait(pthread_barrier_t *);

#ifdef __cplusplus
}
#endif
#endif
