#include <shell.h>
#include <mmu.h>
#include <stdlib.h>

#define MEMORY_CMD_HELP "Show memory information"
#define MEMORY_DUMP_HELP                                                       \
	"Dump the specified memory imformation. Usage: memory dump <vaddr> <len>"
#define MEMORY_TRANSLATE_HELP                                                  \
	"Translate the virtual address to physical address. Usage: memory translate <vaddr>"
#define MEMORY_CMD "memory"
#define MEMORY_DUMP_SUBCMD "dump"
#define MEMORY_TRANSLATE_SUBCMD "translate"

static int memory_dump(struct shell *shell, int argc, char *argv[]) {
	if (!shell) {
		return -1;
	}

	if (argc != 3) {
		shell_show(
			shell,
			"the argument number of subcomand \"%s %s\" is not expected.",
			MEMORY_CMD, argv[0]);
		return -1;
	}

	char *data = (char *)strtoull(argv[1], NULL, 16);
	size_t len = strtoul(argv[2], NULL, 10);

	shell_hexdump(shell, data, len);

	return 0;
}

static int memory_translate(struct shell *shell, int argc,
								   char *argv[]) {
	if (!shell) {
		return -1;
	}

	if (argc != 2) {
		shell_show(
			shell,
			"the argument number of subcomand \"%s %s\" is not expected.",
			MEMORY_CMD, argv[0]);
		return -1;
	}

	virt_addr_t va = strtoull(argv[1], NULL, 16);
	struct mem_domain *domain = kernel_mem_domain_get();
	phys_addr_t pa =
		va_to_pa_translate(domain->arch_mem_domain.pgtable.page_table, va);

	if (!pa) {
		shell_show(shell, "translate failed\n");
	} else {
		shell_show(shell, "the result is 0x%8llx\n", pa);
	}

	return 0;
}

shell_subcmd_set_create(sub_memory,
						shell_subcmd_with_arg(dump, NULL, MEMORY_DUMP_HELP,
											  memory_dump, 3, 0),
						shell_subcmd_with_arg(translate, NULL,
											  MEMORY_TRANSLATE_HELP,
											  memory_translate, 2, 0),
						shell_subcmd_set_end);
shell_cmd_register(memory, &sub_memory, MEMORY_CMD_HELP, NULL);
