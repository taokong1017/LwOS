#ifndef __PFN_H__
#define __PFN_H__

#define PFN_ALIGN(x) (((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
#define PFN_UP(x) (((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x) ((x) >> PAGE_SHIFT)
#define PFN_PHYS(x) ((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x) ((unsigned long)((x) >> PAGE_SHIFT))

#define phys_to_pfn(paddr) PHYS_PFN(paddr)
#define pfn_to_phys(pfn) PFN_PHYS(pfn)
#define PAGE_OFFSET (-((1UL) << (VA_BITS)))

#endif
