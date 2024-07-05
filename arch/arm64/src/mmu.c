
#include <types.h>
#include <pgtable.h>
#include <memory.h>
#include <pgtable_types.h>
#include <pgtable_prot.h>
#include <operate_regs.h>
#include <sys_regs.h>
#include <pfn.h>

#define BIT(nr) (1UL << nr)
#define NO_EXEC_MAPPINGS BIT(2) /* assumes FEAT_HPDS is not used */

extern pgd_t init_pg_dir[];

bool pgattr_change_is_safe(uint64_t old, uint64_t new) {
	/*
	 * The following mapping attributes may be updated in live
	 * kernel mappings without the need for break-before-make.
	 */
	pteval_t mask = PTE_PXN | PTE_RDONLY | PTE_WRITE | PTE_NG;

	/* creating or taking down mappings is always safe */
	if (!pte_valid(__pte(old)) || !pte_valid(__pte(new)))
		return true;

	/* A live entry's pfn should not change */
	if (pte_pfn(__pte(old)) != pte_pfn(__pte(new)))
		return false;

	/* Transitioning from Non-Global to Global is unsafe */
	if (old & ~new &PTE_NG)
		return false;

	/*
	 * Changing the memory type between Normal and Normal-Tagged is safe
	 * since Tagged is considered a permission attribute from the
	 * mismatched attribute aliases perspective.
	 */
	if (((old & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL) ||
		 (old & PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL_TAGGED)) &&
		((new &PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL) ||
		 (new &PTE_ATTRINDX_MASK) == PTE_ATTRINDX(MT_NORMAL_TAGGED)))
		mask |= PTE_ATTRINDX_MASK;

	return ((old ^ new) & ~mask) == 0;
}

static void alloc_init_pte(pmd_t *pmdp, unsigned long addr, unsigned long end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(int), int flags) {
	pmd_t pmd = read_once(*pmdp);
	pte_t *ptep;

	if (pmd_none(pmd)) {
		pmdval_t pmdval = PMD_TYPE_TABLE | PMD_TABLE_UXN;
		phys_addr_t pte_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pmdval |= PMD_TABLE_PXN;
		}
		pte_phys = pgtable_alloc(PAGE_SHIFT);
		__pmd_populate(pmdp, pte_phys, pmdval);
	}

	ptep = (pte_t *)pte_offset_phys(pmdp, addr);
	do {
		pte_t old_pte = __ptep_get(ptep);
		__set_pte(ptep, pfn_pte(__phys_to_pfn(phys), prot));

		if (!pgattr_change_is_safe(pte_val(old_pte),
								   pte_val(__ptep_get(ptep)))) {
			// TODO: handle this case
		}
		phys += PAGE_SIZE;
	} while (ptep++, addr += PAGE_SIZE, addr != end);
}

static void alloc_init_pmd(pud_t *pudp, unsigned long addr, unsigned long end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(int), int flags) {
	unsigned long next;
	pud_t pud = read_once(*pudp);
	pmd_t *pmdp;

	if (pud_none(pud)) {
		pudval_t pudval = PUD_TYPE_TABLE | PUD_TABLE_UXN;
		phys_addr_t pmd_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pudval |= PUD_TABLE_PXN;
		}
		pmd_phys = pgtable_alloc(PMD_SHIFT);
		__pud_populate(pudp, pmd_phys, pudval);
	}

	pmdp = (pmd_t *)pmd_offset_phys(pudp, addr);
	do {
		next = pmd_addr_end(addr, end);
		alloc_init_pte(pmdp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pmdp++, addr = next, addr != end);
}

static void alloc_init_pud(p4d_t *p4dp, unsigned long addr, unsigned long end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(int), int flags) {
	unsigned long next;
	p4d_t p4d = read_once(*p4dp);
	pud_t *pudp;

	if (p4d_none(p4d)) {
		p4dval_t p4dval = P4D_TYPE_TABLE | P4D_TABLE_UXN;
		phys_addr_t pud_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			p4dval |= P4D_TABLE_PXN;
		}

		pud_phys = pgtable_alloc(PUD_SHIFT);
		__p4d_populate(p4dp, pud_phys, p4dval);
	}

	pudp = (pud_t *)pud_offset_phys(p4dp, addr);
	do {
		next = pud_addr_end(addr, end);
		alloc_init_pmd(pudp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pudp++, addr = next, addr != end);
}

static void alloc_init_p4d(pgd_t *pgdp, unsigned long addr, unsigned long end,
						   phys_addr_t phys, pgprot_t prot,
						   phys_addr_t (*pgtable_alloc)(int), int flags) {
	unsigned long next;
	pgd_t pgd = read_once(*pgdp);
	p4d_t *p4dp;

	if (pgd_none(pgd)) {
		pgdval_t pgdval = PGD_TYPE_TABLE | PGD_TABLE_UXN;
		phys_addr_t p4d_phys;

		if (flags & NO_EXEC_MAPPINGS) {
			pgdval |= PGD_TABLE_PXN;
		}

		p4d_phys = pgtable_alloc(P4D_SHIFT);
		__pgd_populate(pgdp, p4d_phys, pgdval);
	}

	p4dp = (p4d_t *)p4d_offset_phys(pgdp, addr);
	do {
		next = p4d_addr_end(addr, end);
		alloc_init_pud(p4dp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (p4dp++, addr = next, addr != end);
}

void create_pgd_mapping(pgd_t *pgdir, phys_addr_t phys, unsigned long virt,
						phys_addr_t size, pgprot_t prot,
						phys_addr_t (*pgtable_alloc)(int), int flags) {
	unsigned long addr, end, next;
	pgd_t *pgdp = pgd_offset_pgd(pgdir, virt);

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
		alloc_init_p4d(pgdp, addr, next, phys, prot, pgtable_alloc, flags);
		phys += next - addr;
	} while (pgdp++, addr = next, addr != end);
}

void mmu_enable() {
	uint64_t tcr = 0;
	uint64_t ttbr0 = 0;
	uint64_t sctlr = 0;

	/* Set memory attribute */
	write_mair_el1(MAIR_EL1_SET);

	/* Set translation control */
	tcr = TCR_T0SZ(VA_BITS) | TCR_T1SZ(VA_BITS) | TCR_IRGN_WBWA |
		  TCR_ORGN_WBWA | TCR_SHARED | TCR_TG0_4K | TCR_TG1_4K | TCR_ASID16 |
		  TCR_TBI0 | TCR_A1 | TCR_TBI1 | TCR_TBID1;
	write_tcr_el1(tcr);

	/* Set translation table */
	ttbr0 = (uint64_t)init_pg_dir;
	write_ttbr0_el1(ttbr0);
	isb();

	/* Set system control */
	sctlr = read_sctlr_el1();
	sctlr |= CTLR_EL1_MMU_ON;
	write_sctlr_el1(sctlr);
	isb();
}
