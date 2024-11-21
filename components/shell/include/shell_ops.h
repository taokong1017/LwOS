#ifndef __SHELL_OPS_H__
#define __SHELL_OPS_H__

#include <types.h>

void shell_op_cursor_horiz_move(struct shell *shell, int32_t delta);
void shell_op_cursor_vert_move(struct shell *shell, int32_t delta);
void shell_cursor_next_line_move(struct shell *shell);
void shell_op_cursor_move(struct shell *shell, uint32_t val);
void shell_op_cursor_home_move(struct shell *shell);
void shell_op_cursor_end_move(struct shell *shell);
void shell_op_cursor_position_synchronize(struct shell *shell);
void shell_op_completion_insert(struct shell *shell, const char * compl,
								uint32_t compl_len);
void shell_op_char_insert(struct shell *shell, char data);
bool shell_cmd_is_full_line(struct shell *shell);
void shell_prompt_and_cmd_print(struct shell *shell);
void shell_cmd_print(struct shell *shell);
void shell_vt100_color_set(struct shell *shell, enum shell_vt100_color color);
void shell_vt100_colors_save(const struct shell *shell,
							 struct shell_vt100_colors *color);
void shell_vt100_colors_restore(struct shell *shell,
								const struct shell_vt100_colors color);

#endif
