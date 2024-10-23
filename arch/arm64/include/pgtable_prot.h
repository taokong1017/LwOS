#ifndef __ARM64_PGTABLE_PROT_H__
#define __ARM64_PGTABLE_PROT_H__

#include <sys_regs.h>

#define MT_P_RW_U_RW                                                           \
	(PTE_TYPE_PAGE | PTE_ATTRINDX(MT_NORMAL) | PTE_NS | PTE_AP_ELx |           \
	 PTE_AP_RW | PTE_SH_INNER | PTE_AF | PTE_NG | PTE_NGP | PTE_DBM |          \
	 PTE_PXN | PTE_UXN)
#define MT_P_RW_U_NA                                                           \
	(PTE_TYPE_PAGE | PTE_ATTRINDX(MT_NORMAL) | PTE_NS | PTE_AP_EL_HIGHER |     \
	 PTE_AP_RW | PTE_SH_INNER | PTE_AF | PTE_NG | PTE_NGP | PTE_DBM |          \
	 PTE_PXN | PTE_UXN)
#define MT_P_RO_U_RO                                                           \
	(PTE_TYPE_PAGE | PTE_ATTRINDX(MT_NORMAL) | PTE_NS | PTE_AP_ELx |           \
	 PTE_AP_RO | PTE_SH_INNER | PTE_AF | PTE_NG | PTE_NGP | PTE_DBM |         \
	 PTE_PXN | PTE_UXN)
#define MT_P_RX_U_RX                                                           \
	(PTE_TYPE_PAGE | PTE_ATTRINDX(MT_NORMAL) | PTE_NS | PTE_AP_ELx |           \
	 PTE_AP_RO | PTE_SH_INNER | PTE_AF | PTE_NG | PTE_NGP | PTE_DBM |         \
	 PTE_PX | PTE_UX)

#endif
