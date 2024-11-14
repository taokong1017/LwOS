#include <shell.h>
#include <shell_ops.h>
#include <string.h>

void shell_op_cursor_horiz_move(struct shell *shell, int32_t delta) {
	char dir = delta > 0 ? 'C' : 'D';

	if (delta == 0) {
		return;
	}

	if (delta < 0) {
		delta = -delta;
	}

	shell_show(shell, "\e[%d%c", delta, dir);
}

void shell_cursor_next_line_move(struct shell *shell) {
	shell_show(shell, "\n");
}

void shell_vt100_color_set(struct shell *shell, enum shell_vt100_color color) {
	if (color >= VT100_COLOR_END) {
		return;
	}

	if (shell->shell_context->vt100_context.col.col == color) {
		return;
	}

	shell->shell_context->vt100_context.col.col = color;

	if (color != SHELL_NORMAL) {
		shell_show(shell, "\e[1;3%dm", color);
	} else {
		shell_show(shell, SHELL_VT100_MODESOFF);
	}
}

void shell_vt100_colors_save(const struct shell *shell,
							 struct shell_vt100_colors *color) {
	memcpy(color, &shell->shell_context->vt100_context.col, sizeof(*color));
}
