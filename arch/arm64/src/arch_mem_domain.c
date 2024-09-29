#include <compiler.h>
#include <menuconfig.h>
#include <arch_mem_domain.h>
#include <mmu.h>

void arch_mem_domain_init(struct arch_mem_domain *arch_mem_domain) {
	uint64_t *pgtable = alloc_page_table();
	uint64_t asid = alloc_page_table_asid();

	assert(pgtable != NULL, "Allocate page table failed\n");
	arch_mem_domain->pgtable.page_table = pgtable;
	arch_mem_domain->pgtable.ttbr0 = (uint64_t)pgtable | asid;

	return;
}
