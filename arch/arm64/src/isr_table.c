#include <types.h>
#include <isr_table.h>

static void irq_spurious(const void *uusned);

struct isr_table_entry sw_isr_table[ISR_TABLE_SIZE] = {
	[0 ...(ISR_TABLE_SIZE - 1)] = {(const void *)NULL, (void *)irq_spurious},
};

void isr_install(uint32_t irq, void (*routine)(const void *),
				 const void *param) {
	uint32_t table_idx = irq;

	sw_isr_table[table_idx].arg = param;
	sw_isr_table[table_idx].isr = routine;
}

static void irq_spurious(const void *unused) { (void)unused; }
