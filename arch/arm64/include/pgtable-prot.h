#ifndef __ARM64_PGTABLE_PROT_H__
#define __ARM64_PGTABLE_PROT_H__

/*
 * Software defined PTE bits definition.
 */
#define PTE_WRITE (PTE_DBM)						 /* same as DBM (51) */
#define PTE_SWP_EXCLUSIVE (((pteval_t)(1)) << 2) /* only for swp ptes */
#define PTE_DIRTY (((pteval_t)(1)) << 55)
#define PTE_SPECIAL (((pteval_t)(1)) << 56)
#define PTE_DEVMAP (((pteval_t)(1)) << 57)
#define PTE_PROT_NONE (((pteval_t)(1)) << 58) /* only when !PTE_VALID */

#endif
