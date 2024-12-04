#include <uart_pl011.h>
#include <device.h>

void uart_early_init() {
	UARTREG(UART_REG_BASE, UART_CR) = (1 << 8) | (1 << 0);
}

void uart_poll_out(const struct device *dev, char c) {
	/* Spin while fifo is full */
	while (UARTREG(UART_REG_BASE, UART_FR) & UART_FR_TXFF) {
	}

	UARTREG(UART_REG_BASE, UART_DR) = c;
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