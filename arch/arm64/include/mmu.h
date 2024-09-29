#ifndef __ARM64_MMU_H__
#define __ARM64_MMU_H__

#include <types.h>
#include <pgtable_types.h>
#include <pgtable_hwdef.h>
#include <pgtable.h>
#include <pgtable_prot.h>

#define VM_ASID_BITS 8
#define TTBR_ASID_SHIFT 48

#define NO_EXEC_MAPPINGS BIT(2)

struct mmu_pgtable {
	uint64_t *page_table;
	uint64_t ttbr0;
};

void create_pgd_mapping(pgd_t *pgdir, phys_addr_t phys, virt_addr_t virt,
						size_t size, pgprot_t prot,
						phys_addr_t (*pgtable_alloc)(size_t), int flags);

void mmu_enable(uint64_t ttbr0);

uint64_t *alloc_page_table();

uint64_t alloc_page_table_asid();

#endif
