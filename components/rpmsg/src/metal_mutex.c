#include <metal_mutex.h>
#include <mutex.h>
#include <stdio.h>

void metal_mutex_init(metal_mutex_t *mutex) {
	char name[MUTEX_NAME_LEN] = {0};
	sprintf(name, "rpmsg_mutex_%lu", (uint64_t)mutex);
	mutex_create(name, (mutex_id_t *)mutex);
}

void metal_mutex_deinit(metal_mutex_t *mutex) {
	mutex_destroy(*((mutex_id_t *)mutex));
}

void metal_mutex_acquire(metal_mutex_t *mutex) {
	mutex_take(*((mutex_id_t *)mutex), MUTEX_WAIT_FOREVER);
}

void metal_mutex_release(metal_mutex_t *mutex) {
	mutex_give(*((mutex_id_t *)mutex));
}
