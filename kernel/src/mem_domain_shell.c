#include <shell.h>
#include <pgtable_prot.h>

#define DOMAIN_CMD_HELP "Show all memory domains layout."
#define KERNEL_DOMAIN_SUBCMD_HELP "Show kernel domain layout."
#define USER_DOMAIN_SUBCMD_HELP "Show user domain layout."
#define DOMAIN_CMD "domain"
#define KERNEL_DOMAIN_SUBCMD "kernel"
#define USER_DOMAIN_SUBCMD "user"

extern struct mem_domain kernel_mem_domain;
extern struct mem_domain user_mem_domain;
const char *separator =
	"--------------------------------------------------------------------------------------\n";

static const char *domain_attr_to_string(mem_partition_attr_t attr) {
	const char *attrs_str = NULL;
	switch (attr.attrs) {
	case MT_P_RW_U_RW:
		attrs_str = "PRW|URW";
		break;

	case MT_P_RW_U_NA:
		attrs_str = "PRW";
		break;

	case MT_P_RO_U_RO:
		attrs_str = "PRO|URO";
		break;

	case MT_P_RX_U_RX:
		attrs_str = "PRX|URX";
		break;

	default:
		attrs_str = "unkown";
		break;
	}

	return attrs_str;
}

static int kernel_domain_show(struct shell *shell, int argc, char *argv[]) {
	int32_t partition_num = kernel_mem_domain.partition_num;
	int32_t index = 0;
	struct mem_partition *partition = NULL;

	if (!shell || argc <= 0 || !argv[0]) {
		return -1;
	}

	if (argc != 1) {
		shell_show(shell,
				   "the argument of subcomand \"domain %s\" is not expected.",
				   argv[0]);
		return -1;
	}

	shell_show(shell, "The kernel domain layout is shown as follows.\n");
	shell_show(shell, separator);
	shell_show(shell, "%-8s%-20s%-18s%-18s%-12s%-10s\n", "Index", "Range Name",
			   "Virtual Address", "Physical Address", "Size", "Attribute");
	shell_show(shell, separator);
	for (; index < partition_num; index++) {
		partition = &kernel_mem_domain.partitions[index];
		shell_show(shell, "[%2d]\t%-20s0x%-16llx0x%-16llx0x%-10lx0x%-10s\n",
				   index, partition->name, partition->vaddr, partition->paddr,
				   partition->size, domain_attr_to_string(partition->attr));
	}
	shell_show(shell, separator);

	return 0;
}

#ifdef CONFIG_USER_SPACE
static int user_domain_show(struct shell *shell, int argc, char *argv[]) {
	int32_t partition_num = user_mem_domain.partition_num;
	int32_t index = 0;
	struct mem_partition *partition = NULL;

	if (!shell || argc <= 0 || !argv[0]) {
		return -1;
	}

	if (argc != 1) {
		shell_show(shell,
				   "the argument of subcomand \"domain %s\" is not expected.",
				   argv[0]);
		return -1;
	}

	shell_show(shell, "The user domain layout is shown as follows.\n");
	shell_show(shell, separator);
	shell_show(shell, "%-8s%-20s%-18s%-18s%-12s%-10s\n", "Index", "Range Name",
			   "Virtual Address", "Physical Address", "Size", "Attribute");
	shell_show(shell, separator);
	for (; index < partition_num; index++) {
		partition = &user_mem_domain.partitions[index];
		shell_show(shell, "[%2d]\t%-20s0x%-16llx0x%-16llx0x%-10lx%-10s\n",
				   index, partition->name, partition->vaddr, partition->paddr,
				   partition->size, domain_attr_to_string(partition->attr));
	};
	shell_show(shell, separator);

	return 0;
}
#endif

shell_subcmd_set_create(sub_domain,
						shell_subcmd_with_arg(kernel, NULL,
											  KERNEL_DOMAIN_SUBCMD_HELP,
											  kernel_domain_show, 1, 0),
#ifdef CONFIG_USER_SPACE
						shell_subcmd_with_arg(user, NULL,
											  USER_DOMAIN_SUBCMD_HELP,
											  user_domain_show, 1, 0),
#endif
						shell_subcmd_set_end);
shell_cmd_register(domain, &sub_domain, DOMAIN_CMD_HELP, NULL);
