#ifndef __SHELL_TYPES_H__
#define __SHELL_TYPES_H__

#include <types.h>

enum shell_vt100_color {
	SHELL_VT100_COLOR_BLACK,
	SHELL_VT100_COLOR_RED,
	SHELL_VT100_COLOR_GREEN,
	SHELL_VT100_COLOR_YELLOW,
	SHELL_VT100_COLOR_BLUE,
	SHELL_VT100_COLOR_MAGENTA,
	SHELL_VT100_COLOR_CYAN,
	SHELL_VT100_COLOR_WHITE,
	SHELL_VT100_COLOR_DEFAULT,
	VT100_COLOR_END,
};

struct shell_vt100_colors {
	enum shell_vt100_color col;	  /* Text color */
	enum shell_vt100_color bgcol; /* Background color */
};

struct shell_multiline_cons {
	uint16_t cur_x;		/* horizontal cursor position in edited command line */
	uint16_t cur_x_end; /* horizontal cursor position at the end of command */
	uint16_t cur_y;		/* vertical cursor position in edited command */
	uint16_t cur_y_end; /* vertical cursor position at the end of command */
	uint16_t terminal_heiht; /* terminal screen height */
	uint16_t terminal_width; /* terminal screen width */
	uint8_t name_len;	   /* console name length */
};

struct shell_vt100_context {
	struct shell_multiline_cons cons;
	struct shell_vt100_colors col;
	uint16_t printed_cmd; /* printed commands counter */
};

#endif
