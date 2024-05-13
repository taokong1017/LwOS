#ifndef __ARM64_PGTABLE_H__
#define __ARM64_PGTABLE_H__

#include <pgtable-types.h>
#include <pgtable-hwdef.h>
#include <compiler.h>

static inline pte_t pgd_pte(pgd_t pgd) { return __pte(pgd_val(pgd)); }

static inline pte_t p4d_pte(p4d_t p4d) { return __pte(p4d_val(p4d)); }

static inline pte_t pud_pte(pud_t pud) { return __pte(pud_val(pud)); }

static inline pud_t pte_pud(pte_t pte) { return __pud(pte_val(pte)); }

static inline pmd_t pud_pmd(pud_t pud) { return __pmd(pud_val(pud)); }

static inline pte_t pmd_pte(pmd_t pmd) { return __pte(pmd_val(pmd)); }

static inline pmd_t pte_pmd(pte_t pte) { return __pmd(pte_val(pte)); }

static inline unsigned long pte_index(unsigned long address) {
	return (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
}

static inline unsigned long pmd_index(unsigned long address) {
	return (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
}

static inline unsigned long pud_index(unsigned long address) {
	return (address >> PUD_SHIFT) & (PTRS_PER_PUD - 1);
}

#define pgd_index(a) (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))

static inline pte_t __ptep_get(pte_t *ptep) { return read_once(*ptep); }

static inline void __set_pte(pte_t *ptep, pte_t pte) {
	write_once(*ptep, pte);

	dsb(ishst);
	isb();
}

#define pfn_to_page(pfn) ((void *)((pfn)*PAGE_SIZE))

#define pte_hw_dirty(pte) (pte_write(pte) && !pte_rdonly(pte))
#define pte_sw_dirty(pte) (!!(pte_val(pte) & PTE_DIRTY))
#define pte_dirty(pte) (pte_sw_dirty(pte) || pte_hw_dirty(pte))
#define pte_valid(pte) (!!(pte_val(pte) & PTE_VALID))

#define __pte_to_phys(pte) (pte_val(pte) & PTE_ADDR_LOW)
#define __phys_to_pte_val(phys) (phys)
#define pte_pfn(pte) (__pte_to_phys(pte) >> PAGE_SHIFT)
#define pfn_pte(pfn, prot)                                                     \
	__pte(__phys_to_pte_val((phys_addr_t)(pfn) << PAGE_SHIFT) |                \
		  pgprot_val(prot))
#define pte_none(pte) (!pte_val(pte))
#define __pte_clear(mm, addr, ptep) __set_pte(ptep, __pte(0))
#define pte_page(pte) (pfn_to_page(pte_pfn(pte)))

#define __pmd_to_phys(pmd) __pte_to_phys(pmd_pte(pmd))
#define __phys_to_pmd_val(phys) __phys_to_pte_val(phys)

#define __pud_to_phys(pud) __pte_to_phys(pud_pte(pud))

static inline phys_addr_t pud_page_paddr(pud_t pud) {
	return __pud_to_phys(pud);
}

static inline phys_addr_t pmd_page_paddr(pmd_t pmd) {
	return __pmd_to_phys(pmd);
}

#define pmd_valid(pmd) pte_valid(pmd_pte(pmd))

static inline void set_pmd(pmd_t *pmdp, pmd_t pmd) {
	write_once(*pmdp, pmd);

	if (pmd_valid(pmd)) {
		dsb(ishst);
		isb();
	}
}

static inline void __pmd_populate(pmd_t *pmdp, phys_addr_t ptep,
								  pmdval_t prot) {
	set_pmd(pmdp, __pmd(__phys_to_pmd_val(ptep) | prot));
}

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
#define pmd_none(pmd) (!pmd_val(pmd))

#define __pud_to_phys(pud) __pte_to_phys(pud_pte(pud))
#define __phys_to_pud_val(phys) __phys_to_pte_val(phys)
#define pud_none(pud) (!pud_val(pud))
#define pud_valid(pud) pte_valid(pud_pte(pud))
static inline void set_pud(pud_t *pudp, pud_t pud) {
	write_once(*pudp, pud);

	if (pud_valid(pud)) {
		dsb(ishst);
		isb();
	}
}

static inline void __pud_populate(pud_t *pudp, phys_addr_t pmdp,
								  pudval_t prot) {
	set_pud(pudp, __pud(__phys_to_pud_val(pmdp) | prot));
}

#define pte_offset_phys(dir, addr)                                             \
	(pmd_page_paddr(read_once(*(dir))) + pte_index(addr) * sizeof(pte_t))
#define pmd_pfn(pmd) ((__pmd_to_phys(pmd) & PMD_MASK) >> PAGE_SHIFT)
#define pfn_pmd(pfn, prot)                                                     \
	__pmd(__phys_to_pmd_val((phys_addr_t)(pfn) << PAGE_SHIFT) |                \
		  pgprot_val(prot))
#define mk_pmd(page, prot) pfn_pmd(page_to_pfn(page), prot)

#define pmd_offset_phys(dir, addr)                                             \
	(pud_page_paddr(read_once(*(dir))) + pmd_index(addr) * sizeof(pmd_t))
#define pud_pfn(pud) ((__pud_to_phys(pud) & PUD_MASK) >> PAGE_SHIFT)
#define pfn_pud(pfn, prot)                                                     \
	__pud(__phys_to_pud_val((phys_addr_t)(pfn) << PAGE_SHIFT) |                \
		  pgprot_val(prot))
#define p4d_none(p4d) (!p4d_val(p4d))

#define pgd_none(pgd) (!pgd_val(pgd))
#define __p4d_to_phys(p4d) __pte_to_phys(p4d_pte(p4d))
#define __phys_to_p4d_val(phys) __phys_to_pte_val(phys)
#define __pgd_to_phys(pgd) __pte_to_phys(pgd_pte(pgd))
#define __phys_to_pgd_val(phys) __phys_to_pte_val(phys)
static inline void set_p4d(p4d_t *p4dp, p4d_t p4d) {
	write_once(*p4dp, p4d);
	dsb(ishst);
	isb();
}
static inline void __p4d_populate(p4d_t *p4dp, phys_addr_t pudp,
								  p4dval_t prot) {
	set_p4d(p4dp, __p4d(__phys_to_p4d_val(pudp) | prot));
}

static inline phys_addr_t p4d_page_paddr(p4d_t p4d) {
	return __p4d_to_phys(p4d);
}

static inline phys_addr_t pud_offset_phys(p4d_t *p4dp, unsigned long addr) {
	return p4d_page_paddr(read_once(*p4dp)) + pud_index(addr) * sizeof(pud_t);
}

#define p4d_index(addr) (((addr) >> P4D_SHIFT) & (PTRS_PER_P4D - 1))

static inline void set_pgd(pgd_t *pgdp, pgd_t pgd) {
	write_once(*pgdp, pgd);
	dsb(ishst);
	isb();
}

static inline phys_addr_t pgd_page_paddr(pgd_t pgd) {
	return __pgd_to_phys(pgd);
}

static inline phys_addr_t p4d_offset_phys(pgd_t *pgdp, unsigned long addr) {
	return pgd_page_paddr(read_once(*pgdp)) + p4d_index(addr) * sizeof(p4d_t);
}

static inline void __pgd_populate(pgd_t *pgdp, phys_addr_t p4dp,
								  pgdval_t prot) {
	set_pgd(pgdp, __pgd(__phys_to_pgd_val(p4dp) | prot));
}

#endif
