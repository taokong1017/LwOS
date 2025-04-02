#include <pthread.h>
#include <errno.h>

int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr,
						  size_t stacksize) {
	if (!attr || !stackaddr || stacksize < PAGE_SIZE) {
		return EINVAL;
	}

	attr->stackaddr = stackaddr;
	attr->stacksize = stacksize;
	attr->stackaddr_set = 1;

	return 0;
}
