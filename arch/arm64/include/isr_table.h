#ifndef __ARM64_ISR_TABLE_H__
#define __ARM64_ISR_TABLE_H__

struct isr_table_entry {
	const void *arg;
	void (*isr)(const void *);
};

void isr_install(uint32_t irq, void (*routine)(const void *),
				 const void *param);

#endif
