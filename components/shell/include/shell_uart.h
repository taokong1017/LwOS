#ifndef __SHELL_UART_H__
#define __SHELL_UART_H__

#include <device.h>
#include <ring_buffer.h>
#include <timer.h>
#include <menuconfig.h>
#include <arch_atomic.h>

enum shell_transport_event {
	SHELL_TRANSPORT_RX_RDY,
	SHELL_TRANSPORT_TX_RDY,
};

typedef void (*shell_transport_handler_t)(enum shell_transport_event event,
										  void *context);

struct shell_uart_base {
	const struct device *dev;
	shell_transport_handler_t handler;
	void *context;
	bool block_tx;
};

struct shell_uart_poll {
	struct shell_uart_base base;
	struct ring_buffer rx_ring_buffer;
	uint8_t rx_buffer[CONFIG_SHELL_RX_RING_BUFFER_SIZE];
	timer_id_t rx_timer_id;
};

struct shell_uart_irq {
	struct shell_uart_base base;
	struct ring_buffer tx_ring_buffer;
	struct ring_buffer rx_ring_buffer;
	uint8_t tx_buffer[CONFIG_SHELL_TX_RING_BUFFER_SIZE];
	uint8_t rx_buffer[CONFIG_SHELL_RX_RING_BUFFER_SIZE];
	timer_id_t dtr_timer_id;
	atomic_t tx_busy;
};

const struct shell_transport_ops *shell_uart_transport_ops_get();

#endif
