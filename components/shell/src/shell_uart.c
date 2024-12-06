#include <shell.h>
#include <uart.h>
#include <tick.h>
#include <log.h>

#define IRQ_TX_BUSY 1
#define IRQ_TX_NO_BUSY 0
#define POLL_TIMER_NAME "shell_uart_timer"
#define SHELL_UART_TAG "SHELL_UART"

#ifdef CONFIG_SHELL_UART_IRQ_TYPE
static void uart_irq_rx_handle(const struct device *dev,
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

static void uart_irq_tx_handle(const struct device *dev,
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

static void uart_irq_callback(const struct device *dev, void *data) {
	struct shell_uart_irq *shell_uart_irq = (struct shell_uart_irq *)data;

	if (uart_irq_rx_ready(dev)) {
		uart_irq_rx_handle(dev, shell_uart_irq);
	}

	if (uart_irq_tx_ready(dev)) {
		uart_irq_tx_handle(dev, shell_uart_irq);
	}
}

static bool shell_uart_irq_init(struct shell_transport *transport) {
	struct shell_uart_irq *shell_uart_irq =
		(struct shell_uart_irq *)transport->transport_context;

	ring_buffer_init(&shell_uart_irq->rx_ring_buffer,
					 CONFIG_SHELL_RX_RING_BUFFER_SIZE,
					 (uint8_t *)shell_uart_irq->rx_buffer);
	ring_buffer_init(&shell_uart_irq->tx_ring_buffer,
					 CONFIG_SHELL_TX_RING_BUFFER_SIZE,
					 (uint8_t *)shell_uart_irq->tx_buffer);
	atomic_set(&shell_uart_irq->tx_busy, IRQ_TX_NO_BUSY);
	uart_irq_callback_set(shell_uart_irq->base.dev, uart_irq_callback,
						  shell_uart_irq);
	uart_irq_rx_enable(shell_uart_irq->base.dev);

	return true;
}

static int32_t shell_uart_irq_write(struct shell_transport *transport,
									const char *data, uint32_t size) {
	uint32_t cnt = 0;
	struct shell_uart_irq *shell_uart_irq =
		(struct shell_uart_irq *)transport->transport_context;

	cnt = ring_buffer_put(&shell_uart_irq->tx_ring_buffer,
						  (const uint8_t *)data, size);
	if (atomic_get(&shell_uart_irq->tx_busy) == IRQ_TX_NO_BUSY) {
		atomic_set(&shell_uart_irq->tx_busy, IRQ_TX_BUSY);
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
static void shell_uart_poll_timeout_callback(void *arg) {
	char dummy = 0;
	struct shell_uart_poll *shell_uart_poll = (struct shell_uart_poll *)arg;

	while (uart_poll_in(shell_uart_poll->base.dev, &dummy)) {
		if (ring_buffer_put(&shell_uart_poll->rx_ring_buffer,
							(const uint8_t *)&dummy, 1) == 0) {
			log_info(SHELL_UART_TAG, "Ring buffer is full!\n");
		}
		shell_uart_poll->base.handler(SHELL_TRANSPORT_RX_RDY,
									  shell_uart_poll->base.context);
	}
}

static bool shell_uart_poll_init(struct shell_transport *transport) {
	errno_t ret = OK;
	struct shell_uart_poll *shell_uart_poll =
		(struct shell_uart_poll *)transport->transport_context;

	ring_buffer_init(&shell_uart_poll->rx_ring_buffer,
					 CONFIG_SHELL_RX_RING_BUFFER_SIZE,
					 shell_uart_poll->rx_buffer);
	ret = timer_create(POLL_TIMER_NAME, TIMER_TYPE_PERIODIC,
					   ms2tick(CONFIG_SHELL_UART_TIMER_INTERVAL),
					   shell_uart_poll_timeout_callback, shell_uart_poll,
					   &shell_uart_poll->rx_timer_id);
	assert(ret == OK, "Create shell uart poll timer failed!\n");
	timer_start(shell_uart_poll->rx_timer_id);

	return true;
}

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

static bool shell_uart_init(struct shell_transport *transport,
							struct device *device,
							shell_transport_handler handler, void *context) {
	struct shell_uart_base *shell_uart_base =
		(struct shell_uart_base *)transport->transport_context;

	shell_uart_base->dev = device;
	shell_uart_base->handler = handler;
	shell_uart_base->context = context;

#ifdef CONFIG_SHELL_UART_IRQ_TYPE
	return shell_uart_irq_init(transport);
#else
	return shell_uart_poll_init(transport);
#endif
}

static bool shell_uart_deinit(struct shell_transport *transport) {
	struct shell_uart_base *shell_uart_base =
		(struct shell_uart_base *)transport->transport_context;

	uart_irq_tx_disable(shell_uart_base->dev);
	uart_irq_rx_disable(shell_uart_base->dev);

	return true;
}

static bool shell_uart_enable(struct shell_transport *transport,
							  bool blocking) {
	struct shell_uart_base *shell_uart_base =
		(struct shell_uart_base *)transport->transport_context;

	shell_uart_base->block_tx = blocking;
#ifdef CONFIG_SHELL_UART_IRQ_TYPE
	if (blocking) {
		uart_irq_tx_disable(shell_uart_base->dev);
	}
#endif

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
