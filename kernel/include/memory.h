#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <pfn.h>

/*
 * Memory types available.
 *
 * IMPORTANT: MT_NORMAL must be index 0 since vm_get_page_prot() may 'or' in
 *	      the MT_NORMAL_TAGGED memory type for PROT_MTE mappings. Note
 *	      that protection_map[] only contains MT_NORMAL attributes.
 */
#define MT_NORMAL 0
#define MT_NORMAL_TAGGED 1
#define MT_NORMAL_NC 2
#define MT_DEVICE_nGnRnE 3
#define MT_DEVICE_nGnRE 4

#define __phys_to_pfn(paddr) PHYS_PFN(paddr)
#define __pfn_to_phys(pfn) PFN_PHYS(pfn)

#define _PAGE_OFFSET(va) (-((1UL) << (va)))
#define PAGE_OFFSET (_PAGE_OFFSET(VA_BITS))

#endif