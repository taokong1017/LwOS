#ifndef __ARM64_PGTABLE_H__
#define __ARM64_PGTABLE_H__

#include <pgtable_types.h>
#include <pgtable_hwdef.h>
#include <compiler.h>

#define POWER_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a) POWER_MASK(x, (__typeof__(x))(a)-1)
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)
#define pfn_to_page(pfn) ((void *)((pfn)*PAGE_SIZE))

#define pte_none(pte) (!pte_val(pte))
#define pte_write(pte) (!!(pte_val(pte) & PTE_WRITE))
#define pte_rdonly(pte) (!!(pte_val(pte) & PTE_RDONLY))
#define pte_hw_dirty(pte) (pte_write(pte) && !pte_rdonly(pte))
#define pte_sw_dirty(pte) (!!(pte_val(pte) & PTE_DIRTY))
#define pte_dirty(pte) (pte_sw_dirty(pte) || pte_hw_dirty(pte))
#define pte_valid(pte) (!!(pte_val(pte) & PTE_VALID))

#define phys_to_pte_val(phys) (phys)
#define pfn_pte(pfn, prot)                                                     \
	pte(phys_to_pte_val((phys_addr_t)(pfn) << PAGE_SHIFT) | pgprot_val(prot))
#define pte_to_phys(pte) (pte_val(pte) & PTE_ADDR_LOW)
#define pte_offset_phys(dir, addr)                                             \
	(pmd_page_paddr(read_once(*(dir))) + pte_index(addr) * sizeof(pte_t))

#define pmd_offset_phys(dir, addr)                                             \
	(pud_page_paddr(read_once(*(dir))) + pmd_index(addr) * sizeof(pmd_t))
#define pmd_none(pmd) (!pmd_val(pmd))
#define pmd_valid(pmd) pte_valid(pmd_pte(pmd))

#define pud_none(pud) (!pud_val(pud))
#define pud_valid(pud) pte_valid(pud_pte(pud))

#define p4d_none(p4d) (!p4d_val(p4d))
#define p4d_index(addr) (((addr) >> P4D_SHIFT) & (PTRS_PER_P4D - 1))
#define p4d_to_phys(p4d) pte_to_phys(p4d_pte(p4d))

#define pgd_none(pgd) (!pgd_val(pgd))
#define pgd_index(a) (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_to_phys(pgd) pte_to_phys(pgd_pte(pgd))

#define pmd_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;             \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})
#define pud_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;             \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})
#define p4d_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + P4D_SIZE) & P4D_MASK;             \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})
#define pgd_addr_end(addr, end)                                                \
	({                                                                         \
		unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK;         \
		(__boundary - 1 < (end)-1) ? __boundary : (end);                       \
	})

static inline pte_t pgd_pte(pgd_t pgd) { return pte(pgd_val(pgd)); }

static inline pte_t p4d_pte(p4d_t p4d) { return pte(p4d_val(p4d)); }

static inline pte_t pud_pte(pud_t pud) { return pte(pud_val(pud)); }

static inline pud_t pte_pud(pte_t pte) { return pud(pte_val(pte)); }

static inline pmd_t pud_pmd(pud_t pud) { return pmd(pud_val(pud)); }

static inline pte_t pmd_pte(pmd_t pmd) { return pte(pmd_val(pmd)); }

static inline pmd_t pte_pmd(pte_t pte) { return pmd(pte_val(pte)); }

static inline unsigned long pte_index(unsigned long address) {
	return (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
}

static inline unsigned long pmd_index(unsigned long address) {
	return (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
}

static inline unsigned long pud_index(unsigned long address) {
	return (address >> PUD_SHIFT) & (PTRS_PER_PUD - 1);
}

static inline pte_t ptep_get(pte_t *ptep) { return read_once(*ptep); }

static inline void set_pte(pte_t *ptep, pte_t pte) {
	write_once(*ptep, pte);

	dsb(ishst);
	isb();
}

static inline phys_addr_t pmd_page_paddr(pmd_t pmd) {
	return pte_to_phys(pmd_pte(pmd));
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmd) {
	write_once(*pmdp, pmd);

	if (pmd_valid(pmd)) {
		dsb(ishst);
		isb();
	}
}

static inline void pmd_populate(pmd_t *pmdp, phys_addr_t ptep, pmdval_t prot) {
	set_pmd(pmdp, pmd(phys_to_pte_val(ptep) | prot));
}

static inline phys_addr_t pud_page_paddr(pud_t pud) {
	return pte_to_phys(pud_pte(pud));
}

static inline void set_pud(pud_t *pudp, pud_t pud) {
	write_once(*pudp, pud);

	if (pud_valid(pud)) {
		dsb(ishst);
		isb();
	}
}

static inline void pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot) {
	set_pud(pudp, pud(phys_to_pte_val(pmdp) | prot));
}

static inline void set_p4d(p4d_t *p4dp, p4d_t p4d) {
	write_once(*p4dp, p4d);
	dsb(ishst);
	isb();
}
static inline void p4d_populate(p4d_t *p4dp, phys_addr_t pudp, p4dval_t prot) {
	set_p4d(p4dp, p4d(phys_to_pte_val(pudp) | prot));
}

static inline phys_addr_t p4d_page_paddr(p4d_t p4d) { return p4d_to_phys(p4d); }

static inline phys_addr_t pud_offset_phys(p4d_t *p4dp, unsigned long addr) {
	return p4d_page_paddr(read_once(*p4dp)) + pud_index(addr) * sizeof(pud_t);
}

static inline void set_pgd(pgd_t *pgdp, pgd_t pgd) {
	write_once(*pgdp, pgd);
	dsb(ishst);
	isb();
}

static inline phys_addr_t pgd_page_paddr(pgd_t pgd) { return pgd_to_phys(pgd); }

static inline phys_addr_t p4d_offset_phys(pgd_t *pgdp, unsigned long addr) {
	return pgd_page_paddr(read_once(*pgdp)) + p4d_index(addr) * sizeof(p4d_t);
}

static inline void pgd_populate(pgd_t *pgdp, phys_addr_t p4dp, pgdval_t prot) {
	set_pgd(pgdp, pgd(phys_to_pte_val(p4dp) | prot));
}

static inline pgd_t *pgd_offset_pgd(pgd_t *pgd, unsigned long address) {
	return (pgd + pgd_index(address));
};

#endif
