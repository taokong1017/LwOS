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
