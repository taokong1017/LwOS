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

/*
 * This bit indicates that the entry is present i.e. pmd_page()
 * still points to a valid huge page in memory even if the pmd
 * has been invalidated.
 */
#define PMD_PRESENT_INVALID                                                    \
	(((pteval_t)(1)) << 59) /* only when !PMD_SECT_VALID */

#define _PROT_DEFAULT (PTE_TYPE_PAGE | PTE_AF | PTE_SHARED)
#define _PROT_SECT_DEFAULT (PMD_TYPE_SECT | PMD_SECT_AF | PMD_SECT_S)

#define PROT_DEFAULT (PTE_TYPE_PAGE | PTE_NG | PTE_SHARED | PTE_AF)
#define PROT_SECT_DEFAULT                                                      \
	(PMD_TYPE_SECT | PMD_MAYBE_NG | PMD_MAYBE_SHARED | PMD_SECT_AF)

#define PROT_DEVICE_nGnRnE                                                     \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE |                            \
	 PTE_ATTRINDX(MT_DEVICE_nGnRnE))
#define PROT_DEVICE_nGnRE                                                      \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE |                            \
	 PTE_ATTRINDX(MT_DEVICE_nGnRE))
#define PROT_NORMAL_NC                                                         \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE | PTE_ATTRINDX(MT_NORMAL_NC))
#define PROT_NORMAL                                                            \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE | PTE_ATTRINDX(MT_NORMAL))
#define PROT_NORMAL_TAGGED                                                     \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE |                            \
	 PTE_ATTRINDX(MT_NORMAL_TAGGED))

#define PROT_SECT_DEVICE_nGnRE                                                 \
	(PROT_SECT_DEFAULT | PMD_SECT_PXN | PMD_SECT_UXN |                         \
	 PMD_ATTRINDX(MT_DEVICE_nGnRE))
#define PROT_SECT_NORMAL                                                       \
	(PROT_SECT_DEFAULT | PMD_SECT_PXN | PMD_SECT_UXN | PTE_WRITE |             \
	 PMD_ATTRINDX(MT_NORMAL))
#define PROT_SECT_NORMAL_EXEC                                                  \
	(PROT_SECT_DEFAULT | PMD_SECT_UXN | PMD_ATTRINDX(MT_NORMAL))

#define PAGE_DEFAULT (_PROT_DEFAULT | PTE_ATTRINDX(MT_NORMAL))

#define PAGE_KERNEL (PROT_NORMAL)
#define PAGE_KERNEL_RO ((PROT_NORMAL & ~PTE_WRITE) | PTE_RDONLY)
#define PAGE_KERNEL_ROX ((PROT_NORMAL & ~(PTE_WRITE | PTE_PXN)) | PTE_RDONLY)
#define PAGE_KERNEL_EXEC (PROT_NORMAL & ~PTE_PXN)
#define PAGE_KERNEL_EXEC_CONT ((PROT_NORMAL & ~PTE_PXN) | PTE_CONT)

#define PAGE_SHARED                                                            \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN | PTE_UXN |       \
	 PTE_WRITE)
#define PAGE_SHARED_EXEC                                                       \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN | PTE_WRITE)
#define PAGE_READONLY                                                          \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN | PTE_UXN)
#define PAGE_READONLY_EXEC                                                     \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN)
#define PAGE_EXECONLY (_PAGE_DEFAULT | PTE_RDONLY | PTE_NG | PTE_PXN)

#endif
