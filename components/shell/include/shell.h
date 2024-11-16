#ifndef __SHELL_H__
#define __SHELL_H__

#include <types.h>
#include <general.h>
#include <shell_history.h>
#include <shell_transport_opts.h>
#include <menuconfig.h>
#include <task.h>
#include <shell_printf.h>
#include <shell_vt100.h>
#include <shell_types.h>

#define SHELL_NAME_LEN 32
#define SHELL_TASK_NAME "Shell_Root"
#define shell_root_cmd_section ".shell.root.cmd"
#define shell_sub_cmd_section ".shell.sub.cmd"
#define declare_align(type, align) type __attribute__((aligned(align)))
#define section_used() __attribute__((__used__))
#define section(segment) __attribute__((section(segment)))
#define type_align(type) __alignof__(type)
#define shell_iterable_section(type, varname, secname)                         \
	declare_align(type, type_align(type)) varname section_used()               \
		section(secname)

#define shell_cmd_init(_expr, _syntax, _subcmd, _help, _handler, _mand, _opt)  \
	{                                                                          \
		.syntax = ((_expr) ? (const char *)(#_syntax) : ""),                   \
		.help = ((_expr) ? (const char *)_help : NULL),                        \
		.subcmd = ((const struct shell_entry *)((_expr) ? _subcmd : NULL)),    \
		.handler = ((shell_cmd_handler)((_expr) ? _handler : NULL)), .args = { \
			.mandatory = (_expr) ? _mand : 0,                                  \
			.optional = (_expr) ? _opt : 0                                     \
		}                                                                      \
	}

#define shell_cmd_register_with_args(syntax, subcmd, help, handler, mandatory, \
									 optional)                                 \
	static const struct shell_entry shell_entry_##syntax = shell_cmd_init(     \
		true, syntax, subcmd, help, handler, mandatory, optional);             \
	static const shell_iterable_section(struct shell_cmd_entry,                \
										shell_cmd_##syntax,                    \
										shell_root_cmd_section) = {            \
		.entry = &shell_entry_##syntax,                                        \
	};

#define shell_cmd_register(syntax, subcmd, help, handler)                      \
	shell_cmd_register_with_args(syntax, subcmd, help, handler, 0, 0)

#define shell_subcmd_set_create(name, ...)                                     \
	static const struct shell_entry shell_##name[] = {__VA_ARGS__};            \
	static const struct shell_cmd_entry name section(                          \
		shell_sub_cmd_section) = {.entry = shell_##name};

#define shell_subcmd_with_arg(syntax, subcmd, help, handler, mand, opt)        \
	shell_cmd_init(true, syntax, subcmd, help, handler, mand, opt)

#define shell_subcmd_set_end                                                   \
	{                                                                          \
		NULL, NULL, NULL, NULL, { 0, 0 }                                       \
	}

#define SHELL_NORMAL SHELL_VT100_COLOR_DEFAULT

enum shell_state {
	SHELL_STATE_UNINITIALIZED,
	SHELL_STATE_INITIALIZED,
	SHELL_STATE_ACTIVE,
};

struct shell;
typedef int (*shell_cmd_handler)(const struct shell *shell, int argc,
								 char *argv[]);

struct shell_args {
	uint8_t mandatory;
	uint8_t optional;
};

struct shell_entry {
	const char *syntax;
	const char *help;
	const struct shell_cmd_entry *subcmd;
	shell_cmd_handler handler;
	struct shell_args args;
} ALIGNED(8);

struct shell_cmd_entry {
	const struct shell_entry *entry;
};

struct shell_transport {
	struct shell_transport_ops *transport_ops;
	void *transport_context;
};

struct shell_context {
	char *cur_prompt;
	enum shell_state state;

	struct shell_entry active_cmd;
	struct shell_entry *selected_cmd;

	struct shell_vt100_context vt100_context;

	char cmd_buff[CONFIG_SHELL_CMD_BUFFER_SIZE];
	uint32_t cmd_buffer_length;
	uint32_t cmd_buffer_position;

	char temp_buff[CONFIG_SHELL_CMD_BUFFER_SIZE];
	uint16_t cmd_tmp_buff_len;
};

struct shell {
	char prompt[SHELL_NAME_LEN];

	struct shell_transport *shell_transport;
	struct shell_context *shell_context;

	struct shell_history *shell_history;
	struct shell_printf *shell_printf;

	task_id_t shell_task_id;
};

/* Shell interface */
void shell_show(struct shell *shell, const char *format, ...);
void shell_color_show(struct shell *shell, enum shell_vt100_color color,
					  const char *format, ...);
#endif
