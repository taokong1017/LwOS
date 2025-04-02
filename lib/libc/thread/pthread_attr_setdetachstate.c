#include <pthread.h>
#include <errno.h>

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate) {
	if (!attr) {
		return EINVAL;
	}

	attr->detachstate = detachstate;
	return 0;
}
