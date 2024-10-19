#ifndef __ARM64_PGTABLE_PROT_H__
#define __ARM64_PGTABLE_PROT_H__

#include <sys_regs.h>

#define PTE_WRITE (PTE_DBM)
#define PROT_DEFAULT (PTE_TYPE_PAGE | PTE_NG | PTE_SHARED | PTE_AF)
#define PROT_NORMAL                                                            \
	(PROT_DEFAULT | PTE_PXN | PTE_UXN | PTE_WRITE | PTE_ATTRINDX(MT_NORMAL))
#define PAGE_KERNEL (PROT_NORMAL)
#define PAGE_KERNEL_RO ((PROT_NORMAL & ~PTE_WRITE) | PTE_RDONLY)
#define PAGE_KERNEL_ROX ((PROT_NORMAL & ~(PTE_WRITE | PTE_PXN)) | PTE_RDONLY)
#define PAGE_KERNEL_EXEC (PROT_NORMAL & ~PTE_PXN)

#define PAGE_DEFAULT                                                           \
	(PTE_TYPE_PAGE | PTE_AF | PTE_SHARED | PTE_ATTRINDX(MT_NORMAL))
#define PAGE_SHARED                                                            \
	(PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_UXN | PTE_WRITE)
#define PAGE_SHARED_EXEC                                                       \
	(PAGE_DEFAULT | PTE_USER | PTE_NG | PTE_PXN | PTE_WRITE)
#define PAGE_READONLY                                                          \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN | PTE_UXN)
#define PAGE_READONLY_EXEC                                                     \
	(PAGE_DEFAULT | PTE_USER | PTE_RDONLY | PTE_NG | PTE_PXN)

#endif
