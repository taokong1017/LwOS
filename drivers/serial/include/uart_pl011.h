#ifndef __UART_PL011_H__
#define __UART_PL011_H__

#include <types.h>

#define UART_REG_BASE (0x09000000)
#define UART_REG_SIZE (0x1000)
#define REG32(addr) ((volatile uint32_t *)(uintptr_t)(addr))
#define UARTREG(base, reg) (*REG32((base) + (reg)))
#define UART_FR_TXFF (0x1U << 5)
#define UART_DR 0x0
#define UART_FR 0x18
#define UART_CR 0x30

void uart_early_init();

void uart_putc(char c);

int32_t uart_puts(const char *s, uint32_t len);

#endif
