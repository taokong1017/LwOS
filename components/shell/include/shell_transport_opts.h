#ifndef __SHELL_TRANSPORT_OPTS_H__
#define __SHELL_TRANSPORT_OPTS_H__

#include <types.h>

struct shell_transport;
struct shell_transport_ops {
	int (*init)(struct shell_transport *transport);
	int (*deinit)(struct shell_transport *transport);
	int (*enable)(struct shell_transport *transport, bool enable);
	int (*write)(struct shell_transport *transport, const char *data,
				 uint32_t size);
	int (*read)(struct shell_transport *transport, char *data, uint32_t size);
};

#endif
