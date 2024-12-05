#include <shell.h>
#include <uart.h>

#define IRQ_TX_BUSY 1
#define IRQ_TX_NO_BUSY 0

#ifdef CONFIG_SHELL_UART_IRQ_TYPE
static void uart_rx_handle(const struct device *dev,
						   struct shell_uart_irq *shell_uart_irq) {
	uint8_t *buffer = NULL;
	uint32_t buffer_len = 0;
	uint32_t read_len = 0;
	bool read_ret = false;
	uint8_t dummy = 0;

	do {
		buffer_len =
			ring_buffer_put_claim(&shell_uart_irq->rx_ring_buffer, &buffer,
								  shell_uart_irq->rx_ring_buffer.size);

		if (buffer_len > 0) {
			read_len =
				uart_irq_read(shell_uart_irq->base.dev, buffer, buffer_len);
			if (read_len > 0) {
				read_ret = true;
			}
			ring_buffer_put_finish(&shell_uart_irq->rx_ring_buffer, read_len);
		} else {
			read_len = uart_irq_read(shell_uart_irq->base.dev, &dummy, 1);
		}
	} while (read_len && (buffer_len == read_len));

	if (read_ret) {
		shell_uart_irq->base.handler(SHELL_TRANSPORT_RX_RDY,
									 shell_uart_irq->base.context);
	}
}

static void uart_tx_handle(const struct device *dev,
						   struct shell_uart_irq *shell_uart_irq) {
	uint32_t len = 0;
	const uint8_t *data = NULL;

	len = ring_buffer_get_claim(&shell_uart_irq->tx_ring_buffer,
								(uint8_t **)&data,
								shell_uart_irq->tx_ring_buffer.size);
	if (len) {
		len = uart_irq_write(dev, data, len);
		ring_buffer_get_finish(&shell_uart_irq->tx_ring_buffer, len);
	} else {
		uart_irq_tx_disable(dev);
		atomic_set(&shell_uart_irq->tx_busy, IRQ_TX_NO_BUSY);
	}

	shell_uart_irq->base.handler(SHELL_TRANSPORT_TX_RDY,
								 shell_uart_irq->base.context);
}

static void uart_callback(const struct device *dev, void *data) {
	struct shell_uart_irq *shell_uart_irq = (struct shell_uart_irq *)data;

	if (uart_irq_rx_ready(dev)) {
		uart_rx_handle(dev, shell_uart_irq);
	}

	if (uart_irq_tx_ready(dev)) {
		uart_tx_handle(dev, shell_uart_irq);
	}
}

static int32_t shell_uart_irq_write(struct shell_transport *transport,
									const char *data, uint32_t size) {
	uint32_t cnt = 0;
	struct shell_uart_irq *shell_uart_irq =
		(struct shell_uart_irq *)transport->transport_context;

	cnt = ring_buffer_put(&shell_uart_irq->tx_ring_buffer,
						  (const uint8_t *)data, size);
	if (atomic_set(&shell_uart_irq->tx_busy, IRQ_TX_BUSY)) {
		uart_irq_tx_enable(shell_uart_irq->base.dev);
	}

	return cnt;
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
