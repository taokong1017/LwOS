#include "board_config.h"

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

SECTIONS
{
	.text : {
		. = ALIGN(4);
		__text_start = .;
		*(.text)			/* .text sections (code) */
		*(.text*)			/* .text* sections (code) */
		. = ALIGN(4);
		__text_end = .;
	}

	.rodata : {
		. = ALIGN(4);
		__rodata_start = .;

		*(.rodata)			/* .rodata sections (constants, strings, etc.) */
		*(.rodata*)			/* .rodata* sections (constants, strings, etc.) */
		. = ALIGN(4);
		__rodata_end = .;
	}

	.bss : {
		. = ALIGN(4);
		_sbss = .;
		__bss_start = .; 	/* define a global symbol at bss start */
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}

	.data ALIGN(0x1000):
	{
		. = ALIGN(4);
		_sdata = .;			/* create a global symbol at data start */
		__ram_data_start = _sdata;

		*(.data)			/* .data sections */
		*(.data*)			/* .data* sections */
		. = ALIGN(4);

		_edata = .;			/* define a global symbol at data end */
		__ram_data_end = _edata;
	}
}
