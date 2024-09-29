#ifndef __ARM64_MEM_DOMAIN_H__
#define __ARM64_MEM_DOMAIN_H__

#include <mmu.h>

typedef struct {
	uint64_t attrs;
} mem_partition_attr_t;

struct arch_mem_domain {
	struct mmu_pgtable pgtable;
};

void arch_mem_domain_init(struct arch_mem_domain *arch_mem_domain);

#endif
