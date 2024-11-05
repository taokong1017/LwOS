#ifndef __SHELL_H__
#define __SHELL_H__

#include <types.h>
#include <general.h>

#define SHELL_NAME_LEN 32

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

struct shell {
	char prompt[SHELL_NAME_LEN];
};

typedef int (*shell_cmd_handler)(const struct shell *shell, int argc,
								 char *argv[]);

struct shell_args {
	uint8_t mandatory;
	uint8_t optional;
};

struct shell_entry {
	const char *syntax;
	const char *help;
	const struct shell_entry *subcmd;
	shell_cmd_handler handler;
	struct shell_args args;
} ALIGNED(8);

struct shell_cmd_entry {
	const struct shell_entry *entry;
};

#endif
