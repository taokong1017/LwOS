#ifndef __ARM64_ISR_TABLE_H__
#define __ARM64_ISR_TABLE_H__

#define ISR_TABLE_SIZE 1024

#ifndef __ASSEMBLY__
struct isr_table_entry {
	const void *arg;
	void (*isr)(const void *);
};

void isr_install(uint32_t irq, void (*routine)(const void *),
				 const void *param);
#endif

#endif
