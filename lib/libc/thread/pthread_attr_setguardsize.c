#include <pthread.h>
#include <errno.h>

int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize) {
	if (!attr) {
		return EINVAL;
	}

	attr->guardsize = guardsize;
	return 0;
}
