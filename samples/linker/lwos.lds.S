OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

#include <menuconfig.h>

SECTIONS
{
	. = 0x40000000;
	.text ALIGN(0x1000):
	{
		__text_start = .;	/* define a global symbol at text start */

		*(.init.text)		/* .init.text sections (code) */
		*(.text)			/* .text sections (code) */
		*(.text*)			/* .text* sections (code) */

		__text_end = .;		/* define a global symbol at text end */
	}

	.rodata ALIGN(0x1000):
	{
		__rodata_start = .;	/* define a global symbol at rodata start */

		*(.rodata)			/* .rodata sections (constants, strings, etc.) */
		*(.rodata*)			/* .rodata* sections (constants, strings, etc.) */

		__rodata_end = .;	/* define a global symbol at rodata end */
	}

	.bss ALIGN(0x1000):
	{
		__bss_start = .; 	/* define a global symbol at bss start */

		*(.bss)				/* .bss sections */
		*(.bss*)			/* .bss* sections */

		__bss_end = .;		/* define a global symbol at bss end */
	}

	.data ALIGN(0x1000):
	{
		__data_start = .;	/* define a global symbol at data start */

		*(.data)			/* .data sections */
		*(.data*)			/* .data* sections */

		__data_end = .;		/* define a global symbol at data end */
	}

	. = ALIGN(0x1000);
	__page_table_pool_start = .;
	. += 0x2000000;
	__page_table_pool_end = .;

	. = ALIGN(0x1000);
	__kernel_stack_start = .;
	. += (CONFIG_KERNEL_STACK_SIZE * CONFIG_CPUS_MAX_NUM);
	__kernel_stack_end = .;

	. = ALIGN(0x1000);
	__interrupt_stack_start = .;
	. += (CONFIG_INTERRUPT_STACK_SIZE * CONFIG_CPUS_MAX_NUM);
	__interrupt_stack_end = .;

	.app_1 ALIGN(0x1000):
	{
		__app_1_bss_start = .;	/* define a global symbol at app bss start */
		*(.app_1_bss)			/* app bss sections */
		__app_1_bss_end = .;	/* define a global symbol at app bss end */

		. = ALIGN(0x1000);
		__app_1_data_start = .;	/* define a global symbol at app data start */
		*(.app_1_data)			/* app data sections */
		__app_1_data_end = .;	/* define a global symbol at app data end */
	}

	.app_2 ALIGN(0x1000):
	{
		__app_2_bss_start = .;	/* define a global symbol at app bss start */
		*(.app_2_bss)			/* app bss sections */
		__app_2_bss_end = .;	/* define a global symbol at app bss end */

		. = ALIGN(0x1000);
		__app_2_data_start = .;	/* define a global symbol at app data start */
		*(.app_2_data)			/* app data sections */
		__app_2_data_end = .;	/* define a global symbol at app data end */
	}
}
