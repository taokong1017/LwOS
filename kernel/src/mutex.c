#include <mutex.h>

errno_t mutex_create(const char *name, mutex_id_t *id) { return OK; }

errno_t mutex_destroy(mutex_id_t id) { return OK; }

errno_t mutex_take(mutex_id_t id, uint32_t timeout) { return OK; }

errno_t mutex_give(mutex_id_t id) { return OK; }