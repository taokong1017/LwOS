#ifndef __ARM64_PGTABLE_HWDEF_H__
#define __ARM64_PGTABLE_HWDEF_H__

#include <pgtable-types.h>

/*
 * 4KB  Granule translate pagetable：
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 * |63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 * |                 |         |         |         |         |
 * |                 |         |         |         |         v
 * |                 |         |         |         |   [11:0]  Page Shift
 * |                 |         |         |         +-> [20:12] L3 index  PTE
 * |                 |         |         +-----------> [29:21] L2 index  PMD
 * |                 |         +---------------------> [38:30] L1 index  PUD
 * |                 +-------------------------------> [47:39] L0 index  L4P
 * <------- PGD
 * +-------------------------------------------------> [63] TTBR0/1
 */
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
/*
 * Number of page-table levels required to address 'va_bits' wide
 * address, without section mapping. We resolve the top (va_bits - PAGE_SHIFT)
 * bits with (PAGE_SHIFT - 3) bits at each page table level. Hence:
 *
 *  levels = DIV_ROUND_UP((va_bits - PAGE_SHIFT), (PAGE_SHIFT - 3))
 *
 * where DIV_ROUND_UP(n, d) => (((n) + (d) - 1) / (d))
 *
 * We cannot include linux/kernel.h which defines DIV_ROUND_UP here
 * due to build issues. So we open code DIV_ROUND_UP here:
 *
 *    ((((va_bits) - PAGE_SHIFT) + (PAGE_SHIFT - 3) - 1) / (PAGE_SHIFT - 3))
 *
 * which gets simplified as :
 */
#define PGTABLE_LEVELS(va_bits) (((va_bits)-4) / (PAGE_SHIFT - 3))

/*
 * Size mapped by an entry at level n ( -1 <= n <= 3)
 * We map (PAGE_SHIFT - 3) at all translation levels and PAGE_SHIFT bits
 * in the final page. The maximum number of translation levels supported by
 * the architecture is 5. Hence, starting at level n, we have further
 * ((4 - n) - 1) levels of translation excluding the offset within the page.
 * So, the total number of bits mapped by an entry at level n is :
 *
 *  ((4 - n) - 1) * (PAGE_SHIFT - 3) + PAGE_SHIFT
 *
 * Rearranging it a bit we get :
 *   (4 - n) * (PAGE_SHIFT - 3) + 3
 */
#define PGTABLE_LEVEL_SHIFT(n) ((PAGE_SHIFT - 3) * (4 - (n)) + 3)

#define PTRS_PER_PTE (1 << (PAGE_SHIFT - 3))

/*
 * PMD_SHIFT determines the size a level 2 page table entry can map.
 */
#define PMD_SHIFT PGTABLE_LEVEL_SHIFT(2)
#define PMD_SIZE (1UL << PMD_SHIFT)
#define PMD_MASK (~(PMD_SIZE - 1))
#define PTRS_PER_PMD (1 << (PAGE_SHIFT - 3))

/*
 * PUD_SHIFT determines the size a level 1 page table entry can map.
 */
#define PUD_SHIFT PGTABLE_LEVEL_SHIFT(1)
#define PUD_SIZE (1UL << PUD_SHIFT)
#define PUD_MASK (~(PUD_SIZE - 1))
#define PTRS_PER_PUD (1 << (PAGE_SHIFT - 3))

#define P4D_SHIFT PGTABLE_LEVEL_SHIFT(0)
#define P4D_SIZE (1UL << P4D_SHIFT)
#define P4D_MASK (~(P4D_SIZE - 1))
#define PTRS_PER_P4D (1 << (PAGE_SHIFT - 3))

/*
 * PGDIR_SHIFT determines the size a top-level page table entry can map
 * (depending on the configuration, this level can be -1, 0, 1 or 2).
 */
#define PGDIR_SHIFT PGTABLE_LEVEL_SHIFT(-1)
#define PGDIR_SIZE (1UL << PGDIR_SHIFT)
#define PGDIR_MASK (~(PGDIR_SIZE - 1))
#define PTRS_PER_PGD (1 << (VA_BITS - PGDIR_SHIFT))

/*
 * Hardware page table definitions.
 *
 * Level -1 descriptor (PGD).
 */
#define PGD_TYPE_TABLE (((pgdval_t)(3)) << 0)
#define PGD_TABLE_BIT (((pgdval_t)(1)) << 1)
#define PGD_TYPE_MASK (((pgdval_t)(3)) << 0)
#define PGD_TABLE_PXN (((pgdval_t)(1)) << 59)
#define PGD_TABLE_UXN (((pgdval_t)(1)) << 60)

/*
 * Level 0 descriptor (P4D).
 */
#define P4D_TYPE_TABLE (((p4dval_t)(3)) << 0)
#define P4D_TABLE_BIT (((p4dval_t)(1)) << 1)
#define P4D_TYPE_MASK (((p4dval_t)(3)) << 0)
#define P4D_TYPE_SECT (((p4dval_t)(1)) << 0)
#define P4D_SECT_RDONLY (((p4dval_t)(1)) << 7) /* AP[2] */
#define P4D_TABLE_PXN (((p4dval_t)(1)) << 59)
#define P4D_TABLE_UXN (((p4dval_t)(1)) << 60)

/*
 * Level 1 descriptor (PUD).
 */
#define PUD_TYPE_TABLE (((pudval_t)(3)) << 0)
#define PUD_TABLE_BIT (((pudval_t)(1)) << 1)
#define PUD_TYPE_MASK (((pudval_t)(3)) << 0)
#define PUD_TYPE_SECT (((pudval_t)(1)) << 0)
#define PUD_SECT_RDONLY (((pudval_t)(1)) << 7)
#define PUD_TABLE_PXN (((pudval_t)(1)) << 59)
#define PUD_TABLE_UXN (((pudval_t)(1)) << 60)

/*
 * Level 2 descriptor (PMD).
 */
#define PMD_TYPE_MASK (((pmdval_t)(3)) << 0)
#define PMD_TYPE_TABLE (((pmdval_t)(3)) << 0)
#define PMD_TYPE_SECT (((pmdval_t)(1)) << 0)
#define PMD_TABLE_BIT (((pmdval_t)(1)) << 1)

/*
 * Section
 */
#define PMD_SECT_VALID (((pmdval_t)(1)) << 0)
#define PMD_SECT_USER (((pmdval_t)(1)) << 6)   /* AP[1] */
#define PMD_SECT_RDONLY (((pmdval_t)(1)) << 7) /* AP[2] */
#define PMD_SECT_S (((pmdval_t)(3)) << 8)
#define PMD_SECT_AF (((pmdval_t)(1)) << 10)
#define PMD_SECT_NG (((pmdval_t)(1)) << 11)
#define PMD_SECT_CONT (((pmdval_t)(1)) << 52)
#define PMD_SECT_PXN (((pmdval_t)(1)) << 53)
#define PMD_SECT_UXN (((pmdval_t)(1)) << 54)
#define PMD_TABLE_PXN (((pmdval_t)(1)) << 59)
#define PMD_TABLE_UXN (((pmdval_t)(1)) << 60)

/*
 * AttrIndx[2:0] encoding (mapping attributes defined in the MAIR* registers).
 */
#define PMD_ATTRINDX(t) (((pmdval_t)(t)) << 2)
#define PMD_ATTRINDX_MASK (((pmdval_t)(7)) << 2)

/*
 * Level 3 descriptor (PTE).
 */
#define PTE_VALID (((pteval_t)(1)) << 0)
#define PTE_TYPE_MASK (((pteval_t)(3)) << 0)
#define PTE_TYPE_PAGE (((pteval_t)(3)) << 0)
#define PTE_TABLE_BIT (((pteval_t)(1)) << 1)
#define PTE_USER (((pteval_t)(1)) << 6)	  /* AP[1] */
#define PTE_RDONLY (((pteval_t)(1)) << 7) /* AP[2] */
#define PTE_SHARED (((pteval_t)(3)) << 8) /* SH[1:0], inner shareable */
#define PTE_AF (((pteval_t)(1)) << 10)	  /* Access Flag */
#define PTE_NG (((pteval_t)(1)) << 11)	  /* nG */
#define PTE_GP (((pteval_t)(1)) << 50)	  /* BTI guarded */
#define PTE_DBM (((pteval_t)(1)) << 51)	  /* Dirty Bit Management */
#define PTE_CONT (((pteval_t)(1)) << 52)  /* Contiguous range */
#define PTE_PXN (((pteval_t)(1)) << 53)	  /* Privileged XN */
#define PTE_UXN (((pteval_t)(1)) << 54)	  /* User XN */

#define PTE_ADDR_LOW ((((pteval_t)(1) << (50 - PAGE_SHIFT)) - 1) << PAGE_SHIFT)
#define PTE_ATTRINDX(t) (((pteval_t)(t)) << 2)
#define PTE_ATTRINDX_MASK (((pteval_t)(7)) << 2)

/*
 * TCR flags.
 */
#define TCR_T0SZ_OFFSET 0
#define TCR_T1SZ_OFFSET 16
#define TCR_T0SZ(x) ((64UL - (x)) << TCR_T0SZ_OFFSET)
#define TCR_T1SZ(x) ((64UL - (x)) << TCR_T1SZ_OFFSET)
#define TCR_TxSZ(x) (TCR_T0SZ(x) | TCR_T1SZ(x))
#define TCR_TxSZ_WIDTH 6
#define TCR_T0SZ_MASK (((1UL << TCR_TxSZ_WIDTH) - 1) << TCR_T0SZ_OFFSET)
#define TCR_T1SZ_MASK (((1UL << TCR_TxSZ_WIDTH) - 1) << TCR_T1SZ_OFFSET)

#define TCR_EPD0_SHIFT 7
#define TCR_EPD0_MASK (1UL << TCR_EPD0_SHIFT)
#define TCR_IRGN0_SHIFT 8
#define TCR_IRGN0_MASK (3UL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_NC (0UL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBWA (1UL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WT (2UL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBnWA (3UL << TCR_IRGN0_SHIFT)

#define TCR_EPD1_SHIFT 23
#define TCR_EPD1_MASK (1UL << TCR_EPD1_SHIFT)
#define TCR_IRGN1_SHIFT 24
#define TCR_IRGN1_MASK (3UL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_NC (0UL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBWA (1UL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WT (2UL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBnWA (3UL << TCR_IRGN1_SHIFT)

#define TCR_IRGN_NC (TCR_IRGN0_NC | TCR_IRGN1_NC)
#define TCR_IRGN_WBWA (TCR_IRGN0_WBWA | TCR_IRGN1_WBWA)
#define TCR_IRGN_WT (TCR_IRGN0_WT | TCR_IRGN1_WT)
#define TCR_IRGN_WBnWA (TCR_IRGN0_WBnWA | TCR_IRGN1_WBnWA)
#define TCR_IRGN_MASK (TCR_IRGN0_MASK | TCR_IRGN1_MASK)

#define TCR_ORGN0_SHIFT 10
#define TCR_ORGN0_MASK (3UL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_NC (0UL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBWA (1UL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WT (2UL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBnWA (3UL << TCR_ORGN0_SHIFT)

#define TCR_ORGN1_SHIFT 26
#define TCR_ORGN1_MASK (3UL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_NC (0UL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBWA (1UL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WT (2UL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBnWA (3UL << TCR_ORGN1_SHIFT)

#define TCR_ORGN_NC (TCR_ORGN0_NC | TCR_ORGN1_NC)
#define TCR_ORGN_WBWA (TCR_ORGN0_WBWA | TCR_ORGN1_WBWA)
#define TCR_ORGN_WT (TCR_ORGN0_WT | TCR_ORGN1_WT)
#define TCR_ORGN_WBnWA (TCR_ORGN0_WBnWA | TCR_ORGN1_WBnWA)
#define TCR_ORGN_MASK (TCR_ORGN0_MASK | TCR_ORGN1_MASK)

#define TCR_SH0_SHIFT 12
#define TCR_SH0_MASK (3UL << TCR_SH0_SHIFT)
#define TCR_SH0_INNER (3UL << TCR_SH0_SHIFT)

#define TCR_SH1_SHIFT 28
#define TCR_SH1_MASK (3UL << TCR_SH1_SHIFT)
#define TCR_SH1_INNER (3UL << TCR_SH1_SHIFT)
#define TCR_SHARED (TCR_SH0_INNER | TCR_SH1_INNER)

#define TCR_TG0_SHIFT 14
#define TCR_TG0_MASK (3UL << TCR_TG0_SHIFT)
#define TCR_TG0_4K (0UL << TCR_TG0_SHIFT)
#define TCR_TG0_64K (1UL << TCR_TG0_SHIFT)
#define TCR_TG0_16K (2UL << TCR_TG0_SHIFT)

#define TCR_TG1_SHIFT 30
#define TCR_TG1_MASK (3UL << TCR_TG1_SHIFT)
#define TCR_TG1_16K (1UL << TCR_TG1_SHIFT)
#define TCR_TG1_4K (2UL << TCR_TG1_SHIFT)
#define TCR_TG1_64K (3UL << TCR_TG1_SHIFT)

#define TCR_IPS_SHIFT 32
#define TCR_IPS_MASK (7UL << TCR_IPS_SHIFT)
#define TCR_A1 (1UL << 22)
#define TCR_ASID16 (1UL << 36)
#define TCR_TBI0 (1UL << 37)
#define TCR_TBI1 (1UL << 38)
#define TCR_HA (1UL << 39)
#define TCR_HD (1UL << 40)
#define TCR_TBID1 (1UL << 52)
#define TCR_NFD0 (1UL << 53)
#define TCR_NFD1 (1UL << 54)
#define TCR_E0PD0 (1UL << 55)
#define TCR_E0PD1 (1UL << 56)
#define TCR_TCMA0 (1UL << 57)
#define TCR_TCMA1 (1UL << 58)
#define TCR_DS (1UL << 59)

#endif
