OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

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
	init_pg_dir = .;
	. += 0x6000;
	init_pg_end = .;

	. = ALIGN(0x1000);
	__kernel_stack_start = .;
	. += 0x1000;
	__kernel_stack_end = .;

	. = ALIGN(0x1000);
	__exec_stack_start = .;
	. += 0x1000;
	__exec_stack_end = .;
}
