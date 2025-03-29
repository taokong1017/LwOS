#include <semaphore.h>

sem_t *sem_open(const char *name, int oflag, ...) {
	(void)name;
	(void)oflag;

	return 0;
}
