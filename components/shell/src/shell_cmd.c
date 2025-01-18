#include <shell.h>

#define SHELL_CLEAR_CMD_HELP "Clear screen."
#define SHELL_HISTORY_CMD_HELP "Command history."
#define SHELL_HELP_CMD_HELP "Prints the help message."

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
			&shell->shell_context->temp_buffer_len, SHELL_HISTORY_DOWN);
		if (!his_ret) {
			break;
		}

		if (shell->shell_context->temp_buffer_len == 0) {
			break;
		}
		shell_show(shell, "[%3d] %s\n", index++,
				   shell->shell_context->temp_buffer);
	}

	shell->shell_context->temp_buffer[0] = '\0';
	shell->shell_context->temp_buffer_len = 0;

	return 0;
}

static int cmd_help(struct shell *shell, int argc, char *argv[]) {
	shell_show(shell, "Use the <Tab> button to prompt or auto-complete"
					  " all commands or its subcommands.\n");

	shell_show(shell, "Try to call commands with <-h> or <--help> parameter"
					  " for more information.\n");

	shell_show(shell, "Shell supports following meta-keys:\n"
					  "  Ctrl + (a key from: abcdefknp)\n"
					  "  Alt  + (a key from: bf).\n");

	shell_help_subcmd_print(shell, NULL, "\nAvailable commands:\n");

	return 0;
}

shell_cmd_register(clear, NULL, SHELL_CLEAR_CMD_HELP, cmd_clear);
shell_cmd_register(history, NULL, SHELL_HISTORY_CMD_HELP, cmd_history);
shell_cmd_register(help, NULL, SHELL_HELP_CMD_HELP, cmd_help);
