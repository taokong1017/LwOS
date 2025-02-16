#ifndef __USER_MEM_DOMAIN_H__
#define __USER_MEM_DOMAIN_H__

#include <types.h>
#include <mem_domain.h>

#ifdef CONFIG_USER_SPACE

#define user_section(segment) __attribute__((section(#segment)))
#define user_data_section user_section(.user_data)
#define user_bss_section user_section(.user_bss)
#define user_rodata_section user_section(.user_rodata)

#define user_mem_domain_define()                                               \
	extern char __user_heap_start[];                                           \
	extern char __user_heap_end[];                                             \
	static const struct mem_range user_heap_region = {                         \
		.name = "user_heap",                                                   \
		.start = (void *)&__user_heap_start[0],                                \
		.end = (void *)&__user_heap_end[0],                                    \
		.attrs = MT_P_RW_U_RW,                                                 \
	};                                                                         \
	extern char __user_data_start[];                                           \
	extern char __user_data_end[];                                             \
	static const struct mem_range user_data_region = {                         \
		.name = "user_data",                                                   \
		.start = (void *)&__user_data_start[0],                                \
		.end = (void *)&__user_data_end[0],                                    \
		.attrs = MT_P_RW_U_RW,                                                 \
	};                                                                         \
	extern char __user_bss_start[];                                            \
	extern char __user_bss_end[];                                              \
	static const struct mem_range user_bss_region = {                          \
		.name = "user_bss",                                                    \
		.start = (void *)&__user_bss_start[0],                                 \
		.end = (void *)&__user_bss_end[0],                                     \
		.attrs = MT_P_RW_U_RW,                                                 \
	};                                                                         \
	extern char __user_rodata_start[];                                         \
	extern char __user_rodata_end[];                                           \
	static const struct mem_range user_rodata_region = {                       \
		.name = "user_rodata",                                                 \
		.start = (void *)&__user_rodata_start[0],                              \
		.end = (void *)&__user_rodata_end[0],                                  \
		.attrs = MT_P_RO_U_RO,                                                 \
	};                                                                         \
	struct mem_domain user_mem_domain;

#define user_mem_domain_init()                                                 \
	do {                                                                       \
		extern void uheap_init();                                              \
		mem_domain_init(&user_mem_domain, "user_domain");                      \
		mem_domain_kernel_ranges_copy(&user_mem_domain);                       \
		mem_domain_ranges_add(&user_mem_domain,                                \
							  (struct mem_range *)&user_heap_region, 1);       \
		if (user_data_region.start < user_data_region.end) {                   \
			mem_domain_ranges_add(&user_mem_domain,                            \
								  (struct mem_range *)&user_data_region, 1);   \
		}                                                                      \
		if (user_bss_region.start < user_bss_region.end) {                     \
			mem_domain_ranges_add(&user_mem_domain,                            \
								  (struct mem_range *)&user_bss_region, 1);    \
		}                                                                      \
		if (user_rodata_region.start < user_rodata_region.end) {               \
			mem_domain_ranges_add(&user_mem_domain,                            \
								  (struct mem_range *)&user_rodata_region, 1); \
		}                                                                      \
		mem_domain_set_up(&user_mem_domain);                                   \
		uheap_init();                                                          \
	} while (0)

#endif
#endif
