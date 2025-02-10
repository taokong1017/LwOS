#ifndef __APP_MEM_DOMAIN_H__
#define __APP_MEM_DOMAIN_H__

#include <types.h>
#include <mem_domain.h>

#ifdef CONFIG_USER_SPACE

#define user_section(segment) __attribute__((section(#segment)))
#define user_data_section user_section(.user_data)
#define user_bss_section user_section(.user_bss)

#define user_mem_domain_define()                                               \
	extern char __user_data_start[];                                           \
	extern char __user_data_end[];                                             \
	static const struct mem_range user_data_region = {                         \
		.start = (void *)&__user_data_start[0],                                \
		.end = (void *)&__user_data_end[0],                                    \
		.attrs = MT_P_RW_U_RW,                                                 \
	};                                                                         \
	extern char __user_bss_start[];                                            \
	extern char __user_bss_end[];                                              \
	static const struct mem_range user_bss_region = {                          \
		.start = (void *)&__user_bss_start[0],                                 \
		.end = (void *)&__user_bss_end[0],                                     \
		.attrs = MT_P_RW_U_RW,                                                 \
	};                                                                         \
	struct mem_domain user_mem_domain;

#define user_mem_domain_init()                                                 \
	do {                                                                       \
		mem_domain_init(&user_mem_domain, "user_domain");                      \
		mem_domain_kernel_ranges_copy(&user_mem_domain);                       \
		mem_domain_ranges_add(&user_mem_domain,                                \
							  (struct mem_range *)&user_data_region, 1);       \
		mem_domain_ranges_add(&user_mem_domain,                                \
							  (struct mem_range *)&user_bss_region, 1);        \
	} while (0)

#endif
#endif
