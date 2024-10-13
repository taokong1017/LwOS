#ifndef __APP_MEM_DOMAIN_H__
#define __APP_MEM_DOMAIN_H__

#include <types.h>
#include <mem_domain.h>

#ifdef CONFIG_USER_SPACE

#define SECTION(segment) __attribute__((section(#segment)))
#define APP_DATA(name) SECTION(.name##_data)
#define APP_BSS(name) SECTION(.name##_bss)
#define APP_DATA_START(name) __##name##_data_start
#define APP_DATA_END(name) __##name##_data_end
#define APP_BSS_START(name) __##name##_bss_start
#define APP_BSS_END(name) __##name##_bss_end

#define APP_PARTITION_DEFINE(name)                                             \
	extern char APP_DATA_START(name)[];                                        \
	extern char APP_DATA_END(name)[];                                          \
	const struct mem_range name##_data_region = {                              \
		.start = (void *)&APP_DATA_START(name)[0],                             \
		.end = (void *)&APP_DATA_END(name)[0],                                 \
		.attrs = PAGE_SHARED,                                                  \
	};                                                                         \
	extern char APP_BSS_START(name)[];                                         \
	extern char APP_BSS_END(name)[];                                           \
	const struct mem_range name##_bss_region = {                               \
		.start = (void *)&APP_BSS_START(name)[0],                              \
		.end = (void *)&APP_BSS_END(name)[0],                                  \
		.attrs = PAGE_SHARED,                                                  \
	};

#endif
#endif
