#ifndef __SHELL_OPS_H__
#define __SHELL_OPS_H__

#include <types.h>

void shell_op_cursor_horiz_move(struct shell *shell, int32_t delta);
void shell_cursor_next_line_move(struct shell *shell);

void shell_vt100_color_set(struct shell *shell, enum shell_vt100_color color);
void shell_vt100_colors_save(const struct shell *shell,
							 struct shell_vt100_colors *color);
void shell_vt100_colors_restore(struct shell *shell,
								const struct shell_vt100_colors color);

#endif
