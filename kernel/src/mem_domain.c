#include <general.h>
#include <mem_domain.h>
#include <uart_pl011.h>
#include <gic_v2.h>
#include <log.h>
#include <spin_lock.h>
#include <string.h>
#include <operate_regs.h>

#define MEM_DOMAIN_TAG "MEM_DOMAIN"
#define MEM_KERNEL_DOMAIN "Kernel_Domain"

extern char __text_start[];
extern char __text_end[];
extern char __rodata_start[];
extern char __rodata_end[];
extern char __data_start[];
extern char __data_end[];
extern char __shell_cmd_start[];
extern char __shell_cmd_end[];
extern char __bss_start[];
extern char __bss_end[];
extern char __kernel_stack_start[];
extern char __kernel_stack_end[];
extern char __kernel_heap_start[];
extern char __kernel_heap_end[];
extern char __interrupt_stack_start[];
extern char __interrupt_stack_end[];
extern char __page_table_pool_start[];
extern char __page_table_pool_end[];
extern char __application_data_start[];
extern char __application_data_end[];
static char *page_table_pool = __page_table_pool_start;
struct mem_domain kernel_mem_domain = {.name = MEM_KERNEL_DOMAIN,
									   .partition_num = 0};
LIST_HEAD(mem_domain_root);
SPIN_LOCK_DEFINE(mem_domain_locker, "mem_domain_locker");

static struct mem_range kernel_mem_ranges[] = {
	{
		.name = "Kernel_Code",
		.start = (void *)__text_start,
		.end = (void *)__text_end,
		.attrs = MT_P_RX_U_RX,
	},
	{
		.name = "Kernel_Ro_Data",
		.start = (void *)__rodata_start,
		.end = (void *)__rodata_end,
		.attrs = MT_P_RO_U_RO,
	},
	{
		.name = "Kernel_Bss",
		.start = (void *)__bss_start,
		.end = (void *)__bss_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Kernel_Data",
		.start = (void *)__data_start,
		.end = (void *)__data_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Shell",
		.start = (void *)__shell_cmd_start,
		.end = (void *)__shell_cmd_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Page_Table_Pool",
		.start = (void *)__page_table_pool_start,
		.end = (void *)__page_table_pool_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Kernel_Stack",
		.start = (void *)__kernel_stack_start,
		.end = (void *)__kernel_stack_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Kernel_Heap",
		.start = (void *)__kernel_heap_start,
		.end = (void *)__kernel_heap_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "Interrupt_Stack",
		.start = (void *)__interrupt_stack_start,
		.end = (void *)__interrupt_stack_end,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "UART",
		.start = (void *)UART_REG_BASE,
		.end = (void *)UART_REG_BASE + UART_REG_SIZE,
		.attrs = MT_P_RW_U_NA,
	},
	{
		.name = "GICv2",
		.start = (void *)GIC_BASE,
		.end = (void *)GIC_BASE + GIC_SIZE,
		.attrs = MT_P_RW_U_NA,
	},
};

static struct mem_range app_mem_ranges[] = {
#ifdef CONFIG_USER_SPACE
	{
		.name = "Apps_Data",
		.start = (void *)__application_data_start,
		.end = (void *)__application_data_end,
		.attrs = MT_P_RW_U_RW,
	},
#endif
};

static errno_t mem_domain_range_check(struct mem_domain *domain,
									  struct mem_range range) {
	uint32_t i = 0;

	for (i = 0; i < domain->partition_num; i++) {
		if ((((virt_addr_t)range.start < domain->partitions[i].vaddr) &&
			 ((virt_addr_t)range.end > domain->partitions[i].vaddr)) ||
			(((virt_addr_t)range.start <
			  domain->partitions[i].vaddr + domain->partitions[i].size) &&
			 ((virt_addr_t)range.end >
			  domain->partitions[i].vaddr + domain->partitions[i].size))) {
			return ERRNO_MEM_DOMAIN_OVERLAP;
		}
	}

	return OK;
}

static errno_t mem_domain_range_add(struct mem_domain *domain,
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

	if (mem_domain_range_check(domain, range) != OK) {
		spin_unlock(&mem_domain_locker);
		log_err(MEM_DOMAIN_TAG, "Range[%s] is overlap\n", range.name);
		return ERRNO_MEM_DOMAIN_OVERLAP;
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

errno_t mem_domain_ranges_add(struct mem_domain *domain,
							  struct mem_range *ranges, uint32_t rangs_num) {
	uint32_t i = 0;

	if (domain == NULL) {
		log_err(MEM_DOMAIN_TAG, "Domain is NULL\n");
		return ERRNO_MEM_DOMAIN_EMPTY;
	}

	if (ranges == NULL) {
		log_err(MEM_DOMAIN_TAG, "Ranges is NULL\n");
		return ERRNO_MEM_DOMAIN_INVALID_RANGE;
	}

	for (i = 0; i < rangs_num; i++) {
		if (mem_domain_range_add(domain, ranges[i]) != OK) {
			log_err(MEM_DOMAIN_TAG, "Add range[%s] failed\n", ranges[i].name);
			return ERRNO_MEM_DOMAIN_FULL;
		}
	}

	return OK;
}

errno_t mem_domain_kernel_ranges_copy(struct mem_domain *domain) {
	if (domain == NULL) {
		log_err(MEM_DOMAIN_TAG, "Domain is NULL\n");
		return ERRNO_MEM_DOMAIN_EMPTY;
	}

	if (domain == &kernel_mem_domain) {
		log_err(MEM_DOMAIN_TAG, "Domain is kernel domain\n");
		return ERRNO_MEM_DOMAIN_SAME;
	}

	return mem_domain_ranges_add(domain, kernel_mem_ranges,
								 ARRAY_SIZE(kernel_mem_ranges));
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
		pgd_mapping_create((pgd_t *)pgtable->page_table, partition->paddr,
						   partition->vaddr, partition->size,
						   pgprot(partition->attr.attrs), page_table_page_alloc,
						   0);
	}
	spin_unlock(&mem_domain_locker);

	return OK;
}

void kernel_mem_domain_init() {
	mem_domain_init(&kernel_mem_domain, MEM_KERNEL_DOMAIN);
	mem_domain_ranges_add(&kernel_mem_domain, kernel_mem_ranges,
						  ARRAY_SIZE(kernel_mem_ranges));
	mem_domain_ranges_add(&kernel_mem_domain, app_mem_ranges,
						  ARRAY_SIZE(app_mem_ranges));
	mem_domain_set_up(&kernel_mem_domain);
	mmu_enable(kernel_mem_domain.arch_mem_domain.pgtable.ttbr0);

	return;
}

struct mmu_pgtable kernel_mem_domain_page_table_get() {
	return kernel_mem_domain.arch_mem_domain.pgtable;
}

phys_addr_t page_table_page_alloc(size_t size) {
	size_t align_size = PAGE_ALIGN(size);
	void *start_ptr = (void *)page_table_pool;
	void *next_ptr = (void *)page_table_pool + align_size;
	void *end_ptr = (void *)__page_table_pool_end;

	if (next_ptr >= end_ptr) {
		assert(NULL, "Page table alloc failed\n");
		return NULL;
	}
	page_table_pool = next_ptr;
	memset(start_ptr, 0x00, align_size);

	return (phys_addr_t)start_ptr;
}

ttbr_t mem_domain_save() {
	ttbr_t ttbr0_old = read_ttbr0_el1();
	ttbr_t ttbr0_new = kernel_mem_domain_page_table_get().ttbr0;

	write_ttbr0_el1(ttbr0_new);
	isb();

	return ttbr0_old;
}

void mem_domain_restore(ttbr_t ttbr) {
	write_ttbr0_el1(ttbr);
	isb();
}

struct mem_domain *kernel_mem_domain_get() {
	return &kernel_mem_domain;
}
