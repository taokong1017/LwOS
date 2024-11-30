#ifndef __DEVICE_H__
#define __DEVICE_H__

struct device {
	const char *name;
	const void *config;
	const void *driver_api;
	struct device_status *status;
	void *data;
};

struct device_status {
	bool initialized;
};

#endif
