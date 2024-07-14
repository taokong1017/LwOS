#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>
#include <errno.h>
#include <timeout.h>
#include <kernel.h>

#define TIMER_NAME_LEN 32
#define TIMER_INVALID_ID -1
#define TIMER_WAIT_FOREVER WAIT_FOREVER
#define TIMER_NO_WAIT NO_WAIT

/* task error code definition */
#define ERRNO_TIMER_INVALID_NAME ERRNO_OS_ERROR(MOD_ID_TIMER, 0x00)
#define ERRNO_TIMER_INVALID_TYPE ERRNO_OS_ERROR(MOD_ID_TIMER, 0x01)
#define ERRNO_TIMER_INVALID_CB ERRNO_OS_ERROR(MOD_ID_TIMER, 0x02)
#define ERRNO_TIMER_INVALID_ID ERRNO_OS_ERROR(MOD_ID_TIMER, 0x03)
#define ERRNO_TIMER_NO_MEMORY ERRNO_OS_FATAL(MOD_ID_TIMER, 0x04)
#define ERRNO_TIMER_INVALID_INTERVAL ERRNO_OS_ERROR(MOD_ID_TIMER, 0x05)

typedef long timer_id_t;
typedef void(timer_cb)(void *arg);
typedef uint64_t interval_t;

enum timer_status {
	TIMER_STATUS_CREATED,
	TIMER_STATUS_RUNNING,
	TIMER_STATUS_STOPPED
};
enum timer_type { TIMER_TYPE_ONE_SHOT, TIMER_TYPE_PERIODIC };

struct timer {
	timer_id_t id;			   /* timer id */
	char name[TIMER_NAME_LEN]; /* timer name */

	enum timer_status status; /* timer status */
	enum timer_type type;	  /* timer type */
	struct timeout timeout;	  /* timer timeout */
	interval_t ticks;		  /* timer ticks */

	timer_cb *cb; /* timer callback */
	void *arg;	  /* timer callback argument */
};

errno_t timer_create(const char *name, enum timer_type type, interval_t ticks,
					 timer_cb *cb, void *arg, timer_id_t *id);
errno_t timer_start(timer_id_t id);
errno_t timer_stop(timer_id_t id);
errno_t timer_destroy(timer_id_t id);

#endif
