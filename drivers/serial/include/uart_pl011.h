#ifndef __UART_PL011_H__
#define __UART_PL011_H__

#include <types.h>

#define UART_REG_BASE (0x9000000)
#define UART_REG_SIZE (0x1000)
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define REG8(addr) ((volatile uint8_t *)(uintptr_t)(addr))
#define UART_UINT32(base, reg) (*REG32((base) + (reg)))
#define UART_UINT8(base, reg) (*REG8((base) + (reg)))
#define UART_FR_TXFF BIT(5)
#define UART_FR_RXFE BIT(4)
#define UART_DR 0x0
#define UART_FR 0x18
#define UART_CR 0x30

void uart_early_init();

int32_t uart_puts(const char *s, int32_t len);

#endif
