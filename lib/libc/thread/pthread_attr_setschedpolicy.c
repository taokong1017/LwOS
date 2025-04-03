#include <pthread.h>
#include <errno.h>

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) {
	if (!attr) {
		return EINVAL;
	}

	if (policy != SCHED_FIFO) {
		return ENOTSUP;
	}

	attr->schedpolicy = policy;
	return 0;
}
