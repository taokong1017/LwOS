#include <types.h>
#include <limits.h>
#include <pfn.h>
#include <sys_regs.h>
#include <menuconfig.h>
#include <mmu.h>
#include <mem_mgr.h>
#include <uart_pl011.h>
#include <gic_v2.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

void map_range(uint64_t *pte, uint64_t start, uint64_t end, uint64_t pa,
			   pgprot_t prot, int level, pte_t *tbl, bool may_use_cont,
			   uint64_t va_offset) {
	uint64_t protval = pgprot_val(prot) & ~PTE_TYPE_MASK;
	int lshift = (3 - level) * (PAGE_SHIFT - 3);
	uint64_t lmask = (PAGE_SIZE << lshift) - 1;

	start &= PAGE_MASK;
	pa &= PAGE_MASK;

	/* Advance tbl to the entry that covers start */
	tbl += (start >> (lshift + PAGE_SHIFT)) % PTRS_PER_PTE;

	/*
	 * Set the right block/page bits for this level unless we are
	 * clearing the mapping
	 */
	if (protval)
		protval |= (level < 3) ? PMD_TYPE_SECT : PTE_TYPE_PAGE;

	while (start < end) {
		uint64_t next = min((start | lmask) + 1, PAGE_ALIGN(end));

		if (level < 3 && (start | next | pa) & lmask) {
			/*
			 * This chunk needs a finer grained mapping. Create a
			 * table mapping if necessary and recurse.
			 */
			if (pte_none(*tbl)) {
				*tbl =
					pte(phys_to_pte_val(*pte) | PMD_TYPE_TABLE | PMD_TABLE_UXN);
				*pte += PTRS_PER_PTE * sizeof(pte_t);
			}
			map_range(pte, start, next, pa, prot, level + 1,
					  (pte_t *)(pte_to_phys(*tbl) + va_offset), may_use_cont,
					  va_offset);
		} else {
			*tbl = pte(phys_to_pte_val(pa) | protval);
		}
		pa += next - start;
		start = next;
		tbl++;
	}
}

void map_segment(pgd_t *pg_dir, uint64_t *pgd, uint64_t va_offset, void *start,
				 void *end, pgprot_t prot, bool may_use_cont, int root_level) {
	map_range(pgd, ((uint64_t)start + va_offset) & ~PAGE_OFFSET,
			  ((uint64_t)end + va_offset) & ~PAGE_OFFSET, (uint64_t)start, prot,
			  root_level, (pte_t *)pg_dir, may_use_cont, 0);
}
