#include <types.h>
#include <limits.h>
#include <pgtable.h>
#include <memory.h>
#include <pgtable_prot.h>

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
				*tbl = __pte(__phys_to_pte_val(*pte) | PMD_TYPE_TABLE |
							 PMD_TABLE_UXN);
				*pte += PTRS_PER_PTE * sizeof(pte_t);
			}
			map_range(pte, start, next, pa, prot, level + 1,
					  (pte_t *)(__pte_to_phys(*tbl) + va_offset), may_use_cont,
					  va_offset);
		} else {
			*tbl = __pte(__phys_to_pte_val(pa) | protval);
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

/* FixMe：to do map at later */
extern pgd_t init_pg_dir[];
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
extern char __exec_stack_start[];
extern char __exec_stack_end[];
extern void *memset(void *s, int c, size_t count);

void early_kernel_map() {
	uint64_t pgdp = (uint64_t)init_pg_dir + PAGE_SIZE;
	pgprot_t text_prot = PAGE_KERNEL_ROX;
	pgprot_t rodata_prot = PAGE_KERNEL_RO;
	pgprot_t data_prot = PAGE_KERNEL;

	/* use 1:1 relation to map text section, rodata section and data section */
	map_segment(init_pg_dir, &pgdp, 0, __text_start, __text_end, text_prot,
				true, 0);
	map_segment(init_pg_dir, &pgdp, 0, __rodata_start, __rodata_end,
				rodata_prot, false, 0);
	map_segment(init_pg_dir, &pgdp, 0, __data_start, __data_end, data_prot,
				false, 0);
	map_segment(init_pg_dir, &pgdp, 0, __bss_start, __bss_end, data_prot, false,
				0);
	map_segment(init_pg_dir, &pgdp, 0, __kernel_stack_start, __kernel_stack_end,
				data_prot, false, 0);
	map_segment(init_pg_dir, &pgdp, 0, __exec_stack_start, __exec_stack_end,
				data_prot, false, 0);
}
