#include <shell.h>

static bool shell_uart_init(struct shell_transport *transport);
static bool shell_uart_deinit(struct shell_transport *transport);
static bool shell_uart_enable(struct shell_transport *transport, bool enable);
static int32_t shell_uart_write(struct shell_transport *transport,
								const char *data, uint32_t size);
static int32_t shell_uart_read(struct shell_transport *transport, char *data,
							   uint32_t size);

const struct shell_transport_ops uart_transport_ops = {
	.init = shell_uart_init,
	.deinit = shell_uart_deinit,
	.enable = shell_uart_enable,
	.read = shell_uart_read,
	.write = shell_uart_write,
};

static bool shell_uart_init(struct shell_transport *transport) {
	(void)transport;

	return true;
}

static bool shell_uart_deinit(struct shell_transport *transport) {
	(void)transport;

	return true;
}

static bool shell_uart_enable(struct shell_transport *transport, bool enable) {
	(void)transport;
	(void)enable;

	return true;
}

static int32_t shell_uart_write(struct shell_transport *transport,
								const char *data, uint32_t size) {
	(void)transport;
	(void)data;

	return size;
}

static int32_t shell_uart_read(struct shell_transport *transport, char *data,
							   uint32_t size) {
	(void)transport;
	(void)data;

	return size;
}

const struct shell_transport_ops *shell_uart_transport_opt_get() {
	return &uart_transport_ops;
}
