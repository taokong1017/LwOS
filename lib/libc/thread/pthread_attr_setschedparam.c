#include <pthread.h>
#include <errno.h>

int pthread_attr_setschedparam(pthread_attr_t *attr,
							   const struct sched_param *param) {
	if (!attr || !param) {
		return EINVAL;
	}

	attr->schedparam = *param;
	return 0;
}
