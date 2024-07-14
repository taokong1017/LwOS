#include <timer.h>
#include <log.h>
#include <mem.h>
#include <string.h>
#include <tick.h>

#define TIMER_TAG "TIMER"
#define timer2id(timer) ((msgq_id_t)timer)
#define id2timer(id) ((struct timer *)id)

#define is_invalid_type(type)                                                  \
	((type != TIMER_TYPE_ONE_SHOT) && (type != TIMER_TYPE_PERIODIC))
#define is_invalid_pointer(cb) (cb == NULL)

static errno_t timer_param_check(const char *name, enum timer_type type,
								 timer_cb *cb, timer_id_t *id) {
	if (!name) {
		log_err(TIMER_TAG, "name is NULL\n");
		return ERRNO_TIMER_INVALID_NAME;
	}

	if (is_invalid_type(type)) {
		log_err(TIMER_TAG, "invalid timer type\n");
		return ERRNO_TIMER_INVALID_TYPE;
	}

	if (is_invalid_pointer(cb)) {
		log_err(TIMER_TAG, "cb is NULL\n");
		return ERRNO_TIMER_INVALID_CB;
	}

	if (is_invalid_pointer(id)) {
		log_err(TIMER_TAG, "id is NULL\n");
		return ERRNO_TIMER_INVALID_ID;
	}

	return OK;
}

static void timer_timeout_handler(struct timeout *timeout) {
	struct timer *timer = container_of(timeout, struct timer, timeout);
	timer_cb *call_back = timer->cb;
	void *arg = timer->arg;

	if (timer->type == TIMER_TYPE_ONE_SHOT) {
		timer->status = TIMER_STATUS_STOPPED;
	}

	if (call_back) {
		call_back(arg);
	}

	if (timer->type == TIMER_TYPE_PERIODIC) {
		timer->timeout.deadline_ticks += timer->ticks;
		timeout_queue_add(&timer->timeout);
	}
}

errno_t timer_create(const char *name, enum timer_type type, uint64_t ticks,
					 timer_cb *cb, void *arg, timer_id_t *id) {
	errno_t ret = OK;
	struct timer *timer = NULL;

	if ((ret = timer_param_check(name, type, cb, id)) != OK) {
		return ret;
	}

	timer = mem_malloc(sizeof(struct timer));
	if (!timer) {
		log_err(TIMER_TAG, "malloc failed\n");
		return ERRNO_TIMER_NO_MEMORY;
	}

	timer->id = (timer_id_t)timer;
	strncpy(timer->name, name, TIMER_NAME_LEN);
	timer->name[TIMER_NAME_LEN - 1] = '\0';
	timer->status = TIMER_STATUS_CREATED;
	timer->type = type;
	timer->ticks = ticks;
	timer->cb = cb;
	timer->arg = arg;
	timer->timeout.func = timer_timeout_handler;
	INIT_LIST_HEAD(&timer->timeout.node);
	*id = timer->id;

	return OK;
}

errno_t timer_start(timer_id_t id) {
	struct timer *timer = id2timer(id);

	if (!timer || id != timer->id) {
		log_err(TIMER_TAG, "invalid timer id\n");
		return ERRNO_TIMER_INVALID_ID;
	}

	if (timer->status == TIMER_STATUS_RUNNING) {
		log_info(TIMER_TAG, "timer %s is already running\n", timer->name);
		timeout_queue_del(&timer->timeout);
	}

	if (timer->ticks == TIMER_WAIT_FOREVER) {
		log_err(TIMER_TAG, "ticks is 0\n");
		return ERRNO_TIMER_INVALID_INTERVAL;
	}

	timer->status = TIMER_STATUS_RUNNING;
	timer->timeout.deadline_ticks = current_ticks_get() + timer->ticks;
	timeout_queue_add(&timer->timeout);
	log_debug(TIMER_TAG, "timer %s is started\n", timer->name);

	return OK;
}

errno_t timer_stop(timer_id_t id) {
	struct timer *timer = id2timer(id);

	if (!timer || id != timer->id) {
		log_err(TIMER_TAG, "invalid timer id\n");
		return ERRNO_TIMER_INVALID_ID;
	}

	if (timer->status == TIMER_STATUS_STOPPED ||
		timer->status == TIMER_STATUS_CREATED) {
		log_info(TIMER_TAG, "timer %s is not runing\n", timer->name);
		return OK;
	}

	timer->status = TIMER_STATUS_STOPPED;
	timeout_queue_del(&timer->timeout);
	log_debug(TIMER_TAG, "timer %s is stoped\n", timer->name);

	return OK;
}

errno_t timer_destroy(timer_id_t id) {
	struct timer *timer = id2timer(id);

	if (!timer || id != timer->id) {
		log_err(TIMER_TAG, "invalid timer id\n");
		return ERRNO_TIMER_INVALID_ID;
	}

	if (timer->status == TIMER_STATUS_RUNNING) {
		timer_stop(id);
	}

	timer->id = TIMER_INVALID_ID;
	log_debug(TIMER_TAG, "timer %s is destroyed\n", timer->name);
	mem_free(timer);

	return OK;
}
