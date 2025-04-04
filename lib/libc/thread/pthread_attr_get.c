#include <pthread.h>
#include <errno.h>

int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr,
						  size_t *stacksize) {
	if (!attr || !stackaddr || !stacksize) {
		return EINVAL;
	}

	*stackaddr = attr->stackaddr;
	*stacksize = attr->stacksize;
	return attr->stackaddr_set ? 0 : EINVAL;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize) {
	if (!attr || !stacksize) {
		return EINVAL;
	}

	*stacksize = attr->stacksize;
	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate) {
	if (!attr || !detachstate) {
		return EINVAL;
	}

	*detachstate = attr->detachstate;
	return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize) {
	if (!attr || !guardsize) {
		return EINVAL;
	}

	*guardsize = attr->guardsize;
	return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr,
							   struct sched_param *param) {
	if (!attr || !param) {
		return EINVAL;
	}

	*param = attr->schedparam;
	return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy) {
	if (!attr || !policy) {
		return EINVAL;
	}

	*policy = attr->schedpolicy;
	return 0;
}
