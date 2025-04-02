#include <pthread.h>
#include <errno.h>

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
	if (!attr) {
		return EINVAL;
	}

	attr->stacksize = stacksize;
	return 0;
}
