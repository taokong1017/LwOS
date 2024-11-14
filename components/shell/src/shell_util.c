#include <shell_util.h>

void shell_transport_buffer_flush(struct shell *shell) {
	shell_printf_flush(shell->shell_printf);
}
