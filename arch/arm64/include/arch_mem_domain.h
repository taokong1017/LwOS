#ifndef __ARM64_MEM_DOMAIN_H__
#define __ARM64_MEM_DOMAIN_H__

#include <mmu.h>

typedef struct {
	uint32_t attrs;
} mem_partition_attr_t;

struct arch_mem_domain {
	struct mmu_pgtables pgtables;
	struct list_head node;
};

#endif
