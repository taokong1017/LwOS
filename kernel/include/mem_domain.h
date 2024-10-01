#ifndef __MEM_DOMAIN_H__
#define __MEM_DOMAIN_H__

#include <errno.h>
#include <list.h>
#include <menuconfig.h>
#include <mmu.h>
#include <arch_mem_domain.h>

#define MEM_DOMAIN_NAME_LEN 32

#define ERRNO_MEM_DOMAIN_EMPTY ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x00)
#define ERRNO_MEM_DOMAIN_FULL ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x01)
#define ERRNO_MEM_DOMAIN_INVALID_NAME ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x02)
#define ERRNO_MEM_DOMAIN_INVALID_RANGE ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x03)
#define ERRNO_MEM_DOMAIN_SAME ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x04)
#define ERRNO_MEM_DOMAIN_OVERLAP ERRNO_OS_ERROR(MOD_ID_MEM_DOMAIN, 0x05)

struct mem_range {
	char *name;
	void *start;
	void *end;
	uint64_t attrs;
};

struct mem_partition {
	virt_addr_t vaddr;
	phys_addr_t paddr;
	size_t size;
	mem_partition_attr_t attr;
};

struct mem_domain {
	char name[MEM_DOMAIN_NAME_LEN];
	struct arch_mem_domain arch_mem_domain;
	struct mem_partition partitions[CONFIG_MEM_PARTITION_MAX_NUM];
	uint32_t partition_num;
	struct list_head mem_domain_node;
};

errno_t mem_domain_init(struct mem_domain *domain, const char *name);
errno_t mem_domain_ranges_add(struct mem_domain *domain,
							  struct mem_range *ranges, uint32_t rangs_num);
errno_t mem_domain_kernel_ranges_copy(struct mem_domain *domain);
errno_t mem_domain_set_up(struct mem_domain *domain);
struct mmu_pgtable kernel_mem_domain_page_table_get();
phys_addr_t page_table_alloc(size_t size);

#endif
