#ifndef __ARM64_TLB_H__
#define __ARM64_TLB_H__

void tlb_local_flush();
void tlb_all_flush();
void tlb_range_flush(virt_addr_t start, virt_addr_t end);

#endif
