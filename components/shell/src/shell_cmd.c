#include <shell.h>

#define SHELL_CLEAR_HELP "Clear screen."
#define SHELL_HISTORY_HELP "Command history."

static int cmd_clear(struct shell *shell, int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	shell_show(shell, SHELL_VT100_CURSORHOME);
	shell_show(shell, SHELL_VT100_CLEARSCREEN);

	return 0;
}

static int cmd_history(struct shell *shell, int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int32_t index = 0;
	bool his_ret = false;

	while (true) {
		his_ret = shell_history_get(
			shell->shell_history, shell->shell_context->temp_buffer,
			&shell->shell_context->temp_buffer_len, SHELL_HISTORY_UP);
		if (!his_ret) {
			break;
		}

		if (shell->shell_context->temp_buffer_len == 0) {
			break;
		}
		shell_show(shell, "[%3d] %s", index++,
				   shell->shell_context->temp_buffer);
	}

	shell->shell_context->temp_buffer[0] = '\0';
	shell->shell_context->temp_buffer_len = 0;

	return 0;
}

shell_cmd_register(clear, NULL, SHELL_CLEAR_HELP, cmd_clear);
shell_cmd_register(history, NULL, SHELL_HISTORY_HELP, cmd_history);
