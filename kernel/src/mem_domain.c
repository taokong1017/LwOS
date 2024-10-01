#include <general.h>
#include <mem_domain.h>
#include <uart_pl011.h>
#include <gic_v2.h>
#include <log.h>
#include <spin_lock.h>
#include <string.h>

#define MEM_DOMAIN_TAG "MEM_DOMAIN"
#define MEM_KERNEL_DOMAIN "Kernel_Domain"

extern char __text_start[];
extern char __text_end[];
extern char __rodata_start[];
extern char __rodata_end[];
extern char __data_start[];
extern char __data_end[];
extern char __bss_start[];
extern char __bss_end[];
extern char __kernel_stack_start[];
extern char __kernel_stack_end[];
extern char __interrupt_stack_start[];
extern char __interrupt_stack_end[];
extern char __page_table_pool_start[];
extern char __page_table_pool_end[];
static char *page_table_pool = __page_table_pool_start;
static struct mem_domain kernel_mem_domain = {.name = MEM_KERNEL_DOMAIN,
											  .partition_num = 0};
LIST_HEAD(mem_domain_root);
SPIN_LOCK_DEFINE(mem_domain_locker, "mem_domain_locker");

static struct mem_range mem_ranges[] = {
	{
		.name = "Kernel_Code",
		.start = (void *)__text_start,
		.end = (void *)__text_end,
		.attrs = PAGE_KERNEL_ROX,
	},
	{
		.name = "Kernel_Ro_Data",
		.start = (void *)__rodata_start,
		.end = (void *)__rodata_end,
		.attrs = PAGE_KERNEL_RO,
	},
	{
		.name = "Kernel_Bss",
		.start = (void *)__bss_start,
		.end = (void *)__bss_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Kernel_Data",
		.start = (void *)__data_start,
		.end = (void *)__data_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Page_Table_Pool",
		.start = (void *)__page_table_pool_start,
		.end = (void *)__page_table_pool_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Kernel_Stack",
		.start = (void *)__kernel_stack_start,
		.end = (void *)__kernel_stack_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "Interrupt_Stack",
		.start = (void *)__interrupt_stack_start,
		.end = (void *)__interrupt_stack_end,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "UART",
		.start = (void *)UART_REG_BASE,
		.end = (void *)UART_REG_BASE + UART_REG_SIZE,
		.attrs = PAGE_KERNEL,
	},
	{
		.name = "GICv2",
		.start = (void *)GIC_BASE,
		.end = (void *)GIC_BASE + GIC_SIZE,
		.attrs = PAGE_KERNEL,
	},
};

errno_t mem_domain_add_range(struct mem_domain *domain,
							 struct mem_range range) {
	uint32_t i = 0;

	spin_lock(&mem_domain_locker);

	if (domain == NULL) {
		spin_unlock(&mem_domain_locker);
		log_err(MEM_DOMAIN_TAG, "Domain is NULL\n");
		return ERRNO_MEM_DOMAIN_EMPTY;
	}

	if (domain->partition_num > CONFIG_MEM_PARTITION_MAX_NUM) {
		spin_unlock(&mem_domain_locker);
		log_err(MEM_DOMAIN_TAG, "Domain[%s] is full\n",
				!domain->name ? "unkown" : domain->name);
		return ERRNO_MEM_DOMAIN_FULL;
	}

	i = domain->partition_num;
	domain->partitions[i].vaddr = (virt_addr_t)range.start;
	domain->partitions[i].paddr = (phys_addr_t)range.start;
	domain->partitions[i].size = range.end - range.start;
	domain->partitions[i].attr.attrs = range.attrs;
	domain->partition_num++;
	spin_unlock(&mem_domain_locker);

	return OK;
}

errno_t mem_domain_init(struct mem_domain *domain, const char *name) {
	spin_lock(&mem_domain_locker);

	if (domain == NULL) {
		spin_unlock(&mem_domain_locker);
		log_err(MEM_DOMAIN_TAG, "Domain is NULL\n");
		return ERRNO_MEM_DOMAIN_EMPTY;
	}

	if (name == NULL) {
		spin_unlock(&mem_domain_locker);
		log_err(MEM_DOMAIN_TAG, "Domain name is NULL\n");
		return ERRNO_MEM_DOMAIN_INVALID_NAME;
	}

	memset(domain, 0, sizeof(struct mem_domain));
	strncpy(domain->name, name, MEM_DOMAIN_NAME_LEN);
	list_add_tail(&domain->mem_domain_node, &mem_domain_root);
	arch_mem_domain_init(&domain->arch_mem_domain);
	spin_unlock(&mem_domain_locker);

	return OK;
}

errno_t mem_domain_set_up(struct mem_domain *domain) {
	uint32_t i = 0;
	struct mem_partition *partition = NULL;
	struct mmu_pgtable *pgtable = NULL;

	spin_lock(&mem_domain_locker);
	if (domain == NULL) {
		log_err(MEM_DOMAIN_TAG, "Domain is NULL\n");
		return ERRNO_MEM_DOMAIN_EMPTY;
	}

	pgtable = &domain->arch_mem_domain.pgtable;
	for (i = 0; i < domain->partition_num; i++) {
		partition = &domain->partitions[i];
		create_pgd_mapping((pgd_t *)pgtable->page_table, partition->paddr,
						   partition->vaddr, partition->size,
						   pgprot(partition->attr.attrs), page_table_alloc, 0);
	}
	spin_unlock(&mem_domain_locker);

	return OK;
}

void kernel_mem_domain_init() {
	uint32_t i = 0;

	mem_domain_init(&kernel_mem_domain, MEM_KERNEL_DOMAIN);
	for (i = 0; i < ARRAY_SIZE(mem_ranges); i++) {
		mem_domain_add_range(&kernel_mem_domain, mem_ranges[i]);
	}
	mem_domain_set_up(&kernel_mem_domain);
	mmu_enable(kernel_mem_domain.arch_mem_domain.pgtable.ttbr0);

	return;
}

struct mmu_pgtable kernel_mem_domain_page_table_get() {
	return kernel_mem_domain.arch_mem_domain.pgtable;
}

phys_addr_t page_table_alloc(size_t size) {
	size_t align_size = PAGE_ALIGN(size);
	void *start_ptr = (void *)page_table_pool;
	void *next_ptr = (void *)page_table_pool + align_size;
	void *end_ptr = (void *)__page_table_pool_end;

	if (next_ptr >= end_ptr) {
		assert(NULL, "Page table alloc failed\n");
		return NULL;
	}
	page_table_pool = next_ptr;

	return (phys_addr_t)start_ptr;
}
