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

		. = ALIGN(0x1000);
		__text_end = .;		/* define a global symbol at text end */
	}

	.rodata ALIGN(0x1000):
	{
		__rodata_start = .;	/* define a global symbol at rodata start */

		*(.rodata)			/* .rodata sections (constants, strings, etc.) */
		*(.rodata*)			/* .rodata* sections (constants, strings, etc.) */

		. = ALIGN(0x1000);
		__rodata_end = .;	/* define a global symbol at rodata end */
	}

	.bss ALIGN(0x1000):
	{
		__bss_start = .; 	/* define a global symbol at bss start */

		*(.bss)				/* .bss sections */
		*(.bss*)			/* .bss* sections */

		. = ALIGN(0x1000);
		__bss_end = .;		/* define a global symbol at bss end */
	}

	.data ALIGN(0x1000):
	{
		__data_start = .;	/* define a global symbol at data start */

		*(.data)			/* .data sections */
		*(.data*)			/* .data* sections */

		. = ALIGN(0x1000);
		__data_end = .;		/* define a global symbol at data end */
	}

	.shell ALIGN(0x1000):
	{
		__shell_cmd_start = .;	/* define shell command start */

		__shell_root_cmd_start = .;
		*(.shell.root.cmd*)		/* shell root command section */
		__shell_root_cmd_end = .;
		. += (8 * CONFIG_SHELL_CMD_SPACE_SIZE);
		__shell_sub_cmd_start = .;
		*(.shell.sub.cmd*)		/* .shell sub-command section */
		__shell_sub_cmd_end = .;

		. = ALIGN(0x1000);
		__shell_cmd_end = .;	/* define shell command end */
	}

	. = ALIGN(0x1000);
	__page_table_pool_start = .;
	. += 0x400000;
	__page_table_pool_end = .;

	. = ALIGN(0x1000);
	__kernel_stack_start = .;
	. += (CONFIG_KERNEL_STACK_SIZE * CONFIG_CPUS_MAX_NUM);
	. = ALIGN(0x1000);
	__kernel_stack_end = .;

	. = ALIGN(0x1000);
	__interrupt_stack_start = .;
	. += (CONFIG_INTERRUPT_STACK_SIZE * CONFIG_CPUS_MAX_NUM);
	. = ALIGN(0x1000);
	__interrupt_stack_end = .;

#ifdef CONFIG_USER_SPACE
	. = ALIGN(0x1000);
	__application_data_start = .;
	#include <samples/linker/app.ld>
	. = ALIGN(0x1000);
	__application_data_end = .;
#endif
}
