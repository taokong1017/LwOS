#include <uart_pl011.h>

void uart_early_init() {
	UARTREG(UART_REG_BASE, UART_CR) = (1 << 8) | (1 << 0);
}

void uart_putc(char c) {
	/* Spin while fifo is full */
	while (UARTREG(UART_REG_BASE, UART_FR) & UART_FR_TXFF) {
	}

	UARTREG(UART_REG_BASE, UART_DR) = c;
}

int32_t uart_puts(const char *s, uint32_t len) {
	const char *ptr = s;

	if (len == 0 || ptr == NULL) {
		return -1;
	}

	for (; ptr < s + len; ptr++) {
		if (*ptr != '\0') {
			if (*ptr == '\n') {
				uart_putc('\r');
			}
			uart_putc(*ptr);
		}
	}

	return len;
}