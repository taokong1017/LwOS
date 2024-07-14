#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>
#include <errno.h>
#include <timeout.h>

#define TIMER_NAME_LEN 32

typedef long timer_id_t;
typedef void(timer_cb)(void *arg);

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

	timer_cb *cb; /* timer callback */
	void *arg;	  /* timer callback argument */
};

errno_t timer_create(const char *name, enum timer_type type, int32_t interval,
					 timer_cb *cb, void *arg, timer_id_t *id);
errno_t timer_start(timer_id_t id);
errno_t timer_stop(timer_id_t id);
errno_t timer_destroy(timer_id_t id);

#endif
