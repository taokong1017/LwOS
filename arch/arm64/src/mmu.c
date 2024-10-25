#include <types.h>
#include <menuconfig.h>
#include <mem_mgr.h>
#include <mmu.h>
#include <operate_regs.h>
#include <sys_regs.h>
#include <pfn.h>
#include <stdio.h>
#include <spin_lock.h>
#include <log.h>
#include <string.h>

#define MMU_LOCKER "MMU_LOCKER"
#define MMU_TAG "MMU"
#define PAGE_TABLE_COUNT_UNIT BIT(16)

static uint64_t
	page_tables[CONFIG_PAGE_TABLE_MAX_NUM * PAGE_TABLE_ENTRY_SIZE] ALIGNED(
		PAGE_TABLE_ENTRY_SIZE * sizeof(uint64_t)) = {0};
static uint32_t page_tables_used_count[CONFIG_PAGE_TABLE_MAX_NUM] = {0};
SPIN_LOCK_DEFINE(mmu_locker, MMU_LOCKER);
static uint8_t pgtable_next_asid = 1;

static void init_pte_alloc(pmd_t *pmdp, virt_addr_t addr, virt_addr_t end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(size_t), int flags) {
	pmd_t pmd = read_once(*pmdp);
	pte_t *ptep;

	if (pmd_none(pmd)) {
		pmdval_t pmdval = PMD_TYPE_TABLE;
		phys_addr_t pte_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pmdval |= (PMD_TABLE_PXN | PMD_TABLE_UXN);
		}
		pte_phys = pgtable_alloc(PAGE_SIZE);
		pmd_populate(pmdp, pte_phys, pmdval);
	}

	ptep = (pte_t *)pte_offset_phys(pmdp, addr);
	do {
		pte_set(ptep, pfn_pte(phys_to_pfn(phys), prot));
		phys += PAGE_SIZE;
	} while (ptep++, addr += PAGE_SIZE, addr != end);
}

static void init_pmd_alloc(pud_t *pudp, virt_addr_t addr, virt_addr_t end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(size_t), int flags) {
	virt_addr_t next;
	pud_t pud = read_once(*pudp);
	pmd_t *pmdp;

	if (pud_none(pud)) {
		pudval_t pudval = PUD_TYPE_TABLE;
		phys_addr_t pmd_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pudval |= (PUD_TABLE_PXN | PUD_TABLE_UXN);
		}
		pmd_phys = pgtable_alloc(PAGE_SIZE);
		pud_populate(pudp, pmd_phys, pudval);
	}

	pmdp = (pmd_t *)pmd_offset_phys(pudp, addr);
	do {
		next = pmd_addr_end(addr, end);
		init_pte_alloc(pmdp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pmdp++, addr = next, addr != end);
}

static void init_pud_alloc(pgd_t *pgdp, virt_addr_t addr, virt_addr_t end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(size_t), int flags) {
	virt_addr_t next;
	pgd_t pgd = read_once(*pgdp);
	pud_t *pudp;

	if (pgd_none(pgd)) {
		pudval_t pudval = PGD_TYPE_TABLE;
		phys_addr_t pud_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pudval |= PUD_TABLE_PXN | PGD_TABLE_UXN;
		}

		pud_phys = pgtable_alloc(PAGE_SIZE);
		pgd_populate(pgdp, pud_phys, pudval);
	}

	pudp = (pud_t *)pud_offset_phys(pgdp, addr);
	do {
		next = pud_addr_end(addr, end);
		init_pmd_alloc(pudp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pudp++, addr = next, addr != end);
}

void pgd_mapping_create(pgd_t *pgdir, phys_addr_t phys, virt_addr_t virt,
						size_t size, pgprot_t prot,
						phys_addr_t (*pgtable_alloc)(size_t), int flags) {
	virt_addr_t addr, end, next;
	pgd_t *pgdp = pgd_offset_pgd(pgdir, virt);
	assert(pgdir != NULL, "pgdir is NULL\n");

	/*
	 * If the virtual and physical address don't have the same offset
	 * within a page, we cannot map the region as the caller expects.
	 */
	if ((phys ^ virt) & ~PAGE_MASK)
		return;

	phys &= PAGE_MASK;
	addr = virt & PAGE_MASK;
	end = PAGE_ALIGN(virt + size);

	do {
		next = pgd_addr_end(addr, end);
		init_pud_alloc(pgdp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pgdp++, addr = next, addr != end);
}

uint64_t *page_table_alloc() {
	uint64_t *table = NULL;
	uint32_t i = 0;

	for (i = 0; i < CONFIG_PAGE_TABLE_MAX_NUM; i++) {
		if (page_tables_used_count[i] == 0) {
			table = &page_tables[i * PAGE_TABLE_ENTRY_SIZE];
			memset(table, 0x00, PAGE_TABLE_ENTRY_SIZE << 3);
			page_tables_used_count[i] += 1;
			log_debug(MMU_TAG, "allocate table[%d]: %p\n", i, table);
			return table;
		}
	}

	log_err(MMU_TAG, "CONFIG_MEM_PARTITION_NUM, too small");
	return NULL;
}

uint64_t page_table_asid_alloc() {
	uint64_t asid = pgtable_next_asid;

	pgtable_next_asid += 1;
	pgtable_next_asid &= (1 << VM_ASID_BITS) - 1;
	if (pgtable_next_asid == 0) {
		pgtable_next_asid = 1;
	}

	return asid << TTBR_ASID_SHIFT;
}

virt_addr_t phys_to_virt(phys_addr_t phys) { return phys; }

phys_addr_t va_to_pa_translate(uint64_t *pgtable_virt, virt_addr_t va) {
	phys_addr_t pa = 0;
	uint64_t prot = 0;
	pgd_t *pgdir = NULL;
	pgd_t pgd;
	pud_t pud;
	pmd_t pmd;
	pte_t pte;

	if (!pgtable_virt) {
		log_err(MMU_TAG, "pgtable_virt is NULL\n");
		return pa;
	}

	pgdir = (pgd_t *)pgtable_virt;
	pgd = *pgd_offset_pgd(pgdir, va); /* virtual address */
	if (pgd_none(pgd)) {
		log_err(MMU_TAG, "pgd value is NULL\n");
		return pa;
	}

	pud = *((pud_t *)pud_offset_phys(&pgd, va)); /* physical address */
	if (pud_none(pud(phys_to_virt(pud_val(pud))))) {
		log_err(MMU_TAG, "pud value is NULL\n");
		return pa;
	}

	pmd = *((pmd_t *)(pmd_offset_phys(&pud, va))); /* physical address */
	if (pmd_none(pmd(phys_to_virt(pmd_val(pmd))))) {
		log_err(MMU_TAG, "pmd value is NULL\n");
		return pa;
	}

	pte = *((pte_t *)pte_offset_phys(&pmd, va)); /* physical address */
	if (pte_none(pte)) {
		log_err(MMU_TAG, "pte value is NULL\n");
		return pa;
	}

	pa = (pte_val(pte) & PTE_ADDR_LOW) + (va & (~PTE_ADDR_LOW));
	prot = pte_val(pte) & (~PTE_ADDR_LOW);
	log_debug(MMU_TAG, "translate va: 0x%p, pa: 0x%p, prot: 0x%p\n", va, pa,
			  prot);

	return pa;
}

void mmu_enable(uint64_t ttbr0) {
	uint64_t tcr = 0;
	uint64_t sctlr = 0;

	/* Set memory attribute */
	write_mair_el1(MAIR_EL1_SET);

	/* Set translation control */
	tcr = TCR_T0SZ(VA_BITS) | TCR_IRGN_WBWA | TCR_ORGN_WBWA | TCR_SHARED |
		  TCR_TG0_4K | TCR_TG1_4K | TCR_TBI0 | TCR_TBI1 | TCR_EPD1_DISABLE;
	write_tcr_el1(tcr);

	/* Set translation table */
	write_ttbr0_el1(ttbr0);
	isb();

	/* Set system control */
	sctlr = read_sctlr_el1();
	sctlr |= CTLR_EL1_MMU_ON;
	write_sctlr_el1(sctlr);
	isb();
}
