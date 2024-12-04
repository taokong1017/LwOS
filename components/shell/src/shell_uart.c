#include <shell.h>
#include <uart.h>

#ifdef CONFIG_SHELL_UART_IRQ_TYPE
static int32_t shell_uart_irq_write(struct shell_transport *transport,
									const char *data, uint32_t size) {
	(void)transport;
	(void)data;

	return size;
}
static int32_t shell_uart_irq_read(struct shell_transport *transport,
								   char *data, uint32_t size) {
	struct shell_uart_irq *shell_uart_irq =
		(struct shell_uart_irq *)transport->transport_context;

	return ring_buffer_get(&shell_uart_irq->rx_ring_buffer, (uint8_t *)data,
						   size);
}
#else
static int32_t shell_uart_poll_write(struct shell_transport *transport,
									 const char *data, uint32_t size) {
	int32_t index = 0;
	struct shell_uart_poll *shell_uart_poll =
		(struct shell_uart_poll *)transport->transport_context;

	for (index = 0; index < size; index++) {
		uart_poll_out(shell_uart_poll->base.dev, data[index]);
	}
	shell_uart_poll->base.handler(SHELL_TRANSPORT_TX_RDY,
								  shell_uart_poll->base.context);

	return size;
}
static int32_t shell_uart_poll_read(struct shell_transport *transport,
									char *data, uint32_t size) {
	struct shell_uart_poll *shell_uart_poll =
		(struct shell_uart_poll *)transport->transport_context;

	return ring_buffer_get(&shell_uart_poll->rx_ring_buffer, (uint8_t *)data,
						   size);
}
#endif

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
#ifdef CONFIG_SHELL_UART_IRQ_TYPE
	return shell_uart_irq_write(transport, data, size);
#else
	return shell_uart_poll_write(transport, data, size);
#endif
}

static int32_t shell_uart_read(struct shell_transport *transport, char *data,
							   uint32_t size) {
#ifdef CONFIG_SHELL_UART_IRQ_TYPE
	return shell_uart_irq_read(transport, data, size);
#else
	return shell_uart_poll_read(transport, data, size);
#endif
}

const struct shell_transport_ops uart_transport_ops = {
	.init = shell_uart_init,
	.deinit = shell_uart_deinit,
	.enable = shell_uart_enable,
	.read = shell_uart_read,
	.write = shell_uart_write,
};

const struct shell_transport_ops *shell_uart_transport_ops_get() {
	return &uart_transport_ops;
}
