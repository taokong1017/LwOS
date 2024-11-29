#ifndef __SHELL_TRANSPORT_OPTS_H__
#define __SHELL_TRANSPORT_OPTS_H__

#include <types.h>

struct shell_transport;
struct shell_transport_ops {
	bool (*init)(struct shell_transport *transport);
	bool (*deinit)(struct shell_transport *transport);
	bool (*enable)(struct shell_transport *transport, bool enable);
	int32_t (*write)(struct shell_transport *transport, const char *data,
					 uint32_t size);
	int32_t (*read)(struct shell_transport *transport, char *data,
					uint32_t size);
};

#endif
