#ifndef __UART_H__
#define __UART_H__

#include <types.h>
#include <errno.h>
#include <device.h>

enum uart_parity {
	UART_PARITY_NONE, /* No parity */
	UART_PARITY_ODD,  /* Odd parity */
	UART_PARITY_EVEN, /* Even parity */
};

enum uart_stop_bits {
	UART_STOP_BITS_0_5, /* 0.5 stop bit */
	UART_STOP_BITS_1,	/* 1 stop bit */
	UART_STOP_BITS_1_5, /* 1.5 stop bits */
	UART_STOP_BITS_2,	/* 2 stop bits */
};

enum uart_data_bits {
	UART_DATA_BITS_5, /* 5 data bits */
	UART_DATA_BITS_6, /* 6 data bits */
	UART_DATA_BITS_7, /* 7 data bits */
	UART_DATA_BITS_8, /* 8 data bits */
	UART_DATA_BITS_9, /* 9 data bits */
};

enum uart_flow_control {
	UART_FLOW_CTRL_NONE,	/* No flow control */
	UART_FLOW_CTRL_RTS_CTS, /* RTS/CTS flow control */
	UART_FLOW_CTRL_DTR_DSR, /* DTR/DSR flow control */
};

struct uart_config {
	uint32_t baudrate; /* Baudrate setting */
	uint8_t parity;	   /* Parity bit */
	uint8_t stop_bits; /* Stop bits */
	uint8_t data_bits; /* Data bits */
	uint8_t flow_ctrl; /* Flow control setting */
};

typedef void (*uart_irq_cb)(const struct device *dev, void *data);

struct uart_driver_ops {
	int32_t (*poll_read)(const struct device *dev, char *data);
	int32_t (*poll_write)(const struct device *dev, char data);
	errno_t (*error_check)(const struct device *dev);

	int32_t (*irq_write)(const struct device *dev, const uint8_t *tx_data,
						 uint32_t len);
	int32_t (*irq_read)(const struct device *dev, uint8_t *rx_data,
						uint32_t size);
	bool (*irq_tx_enable)(const struct device *dev);
	bool (*irq_tx_disable)(const struct device *dev);
	bool (*irq_tx_ready)(const struct device *dev);
	bool (*irq_tx_complete)(const struct device *dev);
	bool (*irq_rx_enable)(const struct device *dev);
	bool (*irq_rx_disable)(const struct device *dev);
	bool (*irq_rx_ready)(const struct device *dev);
	bool (*irq_err_enable)(const struct device *dev);
	bool (*irq_err_disable)(const struct device *dev);
	bool (*irq_callback_set)(const struct device *dev, uart_irq_cb cb,
							 void *data);
	bool (*irq_is_pending)(const struct device *dev);
};

bool uart_poll_in(const struct device *dev, char *c);
bool uart_poll_out(const struct device *dev, char c);
bool uart_irq_tx_enable(const struct device *dev);
bool uart_irq_tx_disable(const struct device *dev);
bool uart_irq_rx_enable(const struct device *dev);
bool uart_irq_rx_disable(const struct device *dev);
bool uart_irq_rx_ready(const struct device *dev);
bool uart_irq_tx_ready(const struct device *dev);
int32_t uart_irq_read(const struct device *dev, uint8_t *rx_data,
					  const int32_t size);
int32_t uart_irq_write(const struct device *dev, const uint8_t *tx_data,
					   const int32_t size);
bool uart_irq_callback_set(const struct device *dev, uart_irq_cb cb,
						   void *data);

#endif
