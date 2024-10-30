#include <metal_mutex.h>
#include <mutex.h>

void metal_mutex_init(metal_mutex_t *mutex) {
	mutex_create("rpmsg_mutex", (mutex_id_t *)mutex);
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
