#ifndef __ARM64_PGTABLE_H__
#define __ARM64_PGTABLE_H__

#include <pgtable_types.h>
#include <pgtable_hwdef.h>
#include <compiler.h>
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)

#define pte_none(pte) (!pte_val(pte))
#define pte_valid(pte) (!!(pte_val(pte) & PTE_VALID))

#define phys_to_pte_val(phys) (phys)
#define pfn_pte(pfn, prot)                                                     \
	pte(phys_to_pte_val((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))
#define pte_to_phys(pte) (pte_val(pte) & PTE_ADDR_LOW)
#define pte_offset_phys(dir, addr)                                             \
	(pte_to_phys(pmd_pte(read_once(*(dir)))) + pte_index(addr) * sizeof(pte_t))

#define pmd_offset_phys(dir, addr)                                             \
	(pte_to_phys(pud_pte((read_once(*(dir))))) + pmd_index(addr) * sizeof(pmd_t))
#define pmd_none(pmd) (!pmd_val(pmd))
#define pmd_valid(pmd) pte_valid(pmd_pte(pmd))
#define pmd_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;             \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})

#define pud_none(pud) (!pud_val(pud))
#define pud_valid(pud) pte_valid(pud_pte(pud))
#define pud_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;             \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})

#define pgd_none(pgd) (!pgd_val(pgd))
#define pgd_index(a) (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_to_phys(pgd) pte_to_phys(pgd_pte(pgd))
#define pgd_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK;         \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})

static inline pte_t pgd_pte(pgd_t pgd) { return pte(pgd_val(pgd)); }

static inline pte_t pud_pte(pud_t pud) { return pte(pud_val(pud)); }

static inline pud_t pte_pud(pte_t pte) { return pud(pte_val(pte)); }

static inline pmd_t pud_pmd(pud_t pud) { return pmd(pud_val(pud)); }

static inline pte_t pmd_pte(pmd_t pmd) { return pte(pmd_val(pmd)); }

static inline pmd_t pte_pmd(pte_t pte) { return pmd(pte_val(pte)); }

static inline size_t pte_index(virt_addr_t address) {
	return (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
}

static inline size_t pmd_index(virt_addr_t address) {
	return (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
}

static inline size_t pud_index(virt_addr_t address) {
	return (address >> PUD_SHIFT) & (PTRS_PER_PUD - 1);
}

static inline void set_pte(pte_t *ptep, pte_t pte) {
	write_once(*ptep, pte);

	dsb(ishst);
	isb();
}

static inline void pmd_populate(pmd_t *pmdp, phys_addr_t ptep, pmdval_t prot) {
	pmd_t pmd = pmd(phys_to_pte_val(ptep) | prot);

	write_once(*pmdp, pmd);
	if (pmd_valid(pmd)) {
		dsb(ishst);
		isb();
	}
}

static inline void pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot) {
	pud_t pud = pud(phys_to_pte_val(pmdp) | prot);
	write_once(*pudp, pud);

	if (pud_valid(pud)) {
		dsb(ishst);
		isb();
	}
}

static inline phys_addr_t pud_offset_phys(pgd_t *pgdp, unsigned long addr) {
	return pgd_to_phys(read_once(*pgdp)) + pud_index(addr) * sizeof(pud_t);
}

static inline void pgd_populate(pgd_t *pgdp, phys_addr_t pudp, pgdval_t prot) {
	pgd_t pgd = pgd(phys_to_pte_val(pudp) | prot);
	write_once(*pgdp, pgd);
	dsb(ishst);
	isb();
}

static inline pgd_t *pgd_offset_pgd(pgd_t *pgd, unsigned long address) {
	return (pgd + pgd_index(address));
};

#endif
