#include "board_config.h"

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

SECTIONS
{
	.text : {
		. = ALIGN(4);
		__text_start = .;		/* define a global symbol at text start */

		*(.text)				/* .text sections (code) */
		*(.text*)				/* .text* sections (code) */

		. = ALIGN(4);
		__text_end = .;			/* define a global symbol at text end */
	}

	.rodata : {
		. = ALIGN(4);
		__rodata_start = .;		/* define a global symbol at rodata start */

		*(.rodata)				/* .rodata sections (constants, strings, etc.) */
		*(.rodata*)				/* .rodata* sections (constants, strings, etc.) */

		. = ALIGN(4);
		__rodata_end = .;		/* define a global symbol at rodata end */
	}

	.bss : {
		. = ALIGN(4);
		__bss_start = .; 		/* define a global symbol at bss start */

		*(.bss)					/* .bss sections */
		*(.bss*)				/* .bss* sections */

		. = ALIGN(4);
		__bss_end = .;			/* define a global symbol at bss end */
	}

	.data ALIGN(0x1000):
	{
		. = ALIGN(4);
		__data_start = .;		/* define a global symbol at data start */

		*(.data)				/* .data sections */
		*(.data*)				/* .data* sections */

		. = ALIGN(4);
		__data_end = .;		/* define a global symbol at data end */
	}
}