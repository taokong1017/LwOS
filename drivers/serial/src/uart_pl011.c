#include <uart_pl011.h>
#include <device.h>
#include <uart.h>

void uart_early_init() {
	UARTREG(UART_REG_BASE, UART_CR) = (1 << 8) | (1 << 0);
}

bool uart_poll_in(const struct device *dev, char *c) { return true; }

bool uart_poll_out(const struct device *dev, char c) {
	/* Spin while fifo is full */
	while (UARTREG(UART_REG_BASE, UART_FR) & UART_FR_TXFF) {
	}

	UARTREG(UART_REG_BASE, UART_DR) = c;

	return true;
}

int32_t uart_puts(const char *s, int32_t len) {
	const char *ptr = s;

	if (len <= 0 || ptr == NULL) {
		return -1;
	}

	for (; ptr < s + len; ptr++) {
		if (*ptr != '\0') {
			if (*ptr == '\n') {
				uart_poll_out(NULL, '\r');
			}
			uart_poll_out(NULL, *ptr);
		}
	}

	return len;
}

bool uart_irq_tx_enable(const struct device *dev) { return true; }

bool uart_irq_tx_disable(const struct device *dev) { return true; }

bool uart_irq_rx_enable(const struct device *dev) { return true; }

bool uart_irq_rx_disable(const struct device *dev) { return true; }

bool uart_irq_rx_ready(const struct device *dev) { return true; }

bool uart_irq_tx_ready(const struct device *dev) { return true; }

int32_t uart_irq_read(const struct device *dev, uint8_t *rx_data,
					  const int32_t size) {
	return 0;
}

int32_t uart_irq_write(const struct device *dev, const uint8_t *tx_data,
					   const int32_t size) {
	return 0;
}

bool uart_irq_callback_set(const struct device *dev, uart_irq_callback_t cb,
						   void *data) {
	return true;
}
