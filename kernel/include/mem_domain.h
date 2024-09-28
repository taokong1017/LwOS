#ifndef __MEM_DOMAIN_H__
#define __MEM_DOMAIN_H__

#include <list.h>
#include <menuconfig.h>
#include <mmu.h>
#include <arch_mem_domain.h>

struct mem_partition {
	uintptr_t start;
	size_t size;
	mem_partition_attr_t attr;
};

struct mem_domain {
	struct arch_mem_domain arch_mem_domain;
	struct mem_partition partitions[CONFIG_MEM_PARTITION_MAX_NUM];
	uint32_t partition_num;
	struct list_head mem_domain_node;
};

#endif
