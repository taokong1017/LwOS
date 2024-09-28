#include <mem_domain.h>
#include <uart_pl011.h>
#include <gic_v2.h>

struct mem_range {
	char *name;
	void *start;
	void *end;
	uint64_t attrs;
};

extern char __text_start[];
extern char __text_end[];
extern char __rodata_start[];
extern char __rodata_end[];
extern char __data_start[];
extern char __data_end[];
extern char __bss_start[];
extern char __bss_end[];
extern char __kernel_stack_start[];
extern char __kernel_stack_end[];
extern char __interrupt_stack_start[];
extern char __interrupt_stack_end[];

static struct mem_domain kernel_mem_domain;
static struct mem_range mem_ranges[] = {
	{
		.name = "Kernel_Code",
		.start = (void *)__text_start,
		.end = (void *)__text_end,
		.attrs = PAGE_KERNEL_ROX,
	},
	{
		.name = "Kernel_RO_Data",
		.start = (void *)__rodata_start,
		.end = (void *)__rodata_start,
		.attrs = PAGE_KERNEL_RO,
	},
	{
		.name = "Kernel_Data",
		.start = (void *)__data_start,
		.end = (void *)__rodata_start,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Kernel_BSS",
		.start = (void *)__bss_start,
		.end = (void *)__bss_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Kernel_Stack",
		.start = (void *)__kernel_stack_start,
		.end = (void *)__kernel_stack_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Interrupt_Stack",
		.start = (void *)__interrupt_stack_start,
		.end = (void *)__interrupt_stack_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "UART",
		.start = (void *)UART_REG_BASE,
		.end = (void *)UART_REG_BASE + UART_REG_SIZE,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "GICv2",
		.start = (void *)GIC_BASE,
		.end = (void *)GIC_BASE + GIC_SIZE,
		.attrs = PAGE_KERNEL,
	},
};
