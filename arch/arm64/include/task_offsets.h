#ifndef __ARM64_TASK_OFFSETS_H__
#define __ARM64_TASK_OFFSETS_H__

#include <offsets.h>
#include <menuconfig.h>

#define ARCH_TASK_CONTEXT_X0_X1_OFFSET                                         \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X0_X1_OFFSET)
#define ARCH_TASK_CONTEXT_X2_X3_OFFSET                                         \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X2_X3_OFFSET)
#define ARCH_TASK_CONTEXT_X4_X5_OFFSET                                         \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X4_X5_OFFSET)
#define ARCH_TASK_CONTEXT_X6_X7_OFFSET                                         \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X6_X7_OFFSET)
#define ARCH_TASK_CONTEXT_X8_X9_OFFSET                                         \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X8_X9_OFFSET)
#define ARCH_TASK_CONTEXT_X10_X11_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X10_X11_OFFSET)
#define ARCH_TASK_CONTEXT_X12_X13_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X12_X13_OFFSET)
#define ARCH_TASK_CONTEXT_X14_X15_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X14_X15_OFFSET)
#define ARCH_TASK_CONTEXT_X16_X17_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X16_X17_OFFSET)
#define ARCH_TASK_CONTEXT_X18_X19_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X18_X19_OFFSET)
#define ARCH_TASK_CONTEXT_X20_X21_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X20_X21_OFFSET)
#define ARCH_TASK_CONTEXT_X22_X23_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X22_X23_OFFSET)
#define ARCH_TASK_CONTEXT_X24_X25_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X24_X25_OFFSET)
#define ARCH_TASK_CONTEXT_X26_X27_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X26_X27_OFFSET)
#define ARCH_TASK_CONTEXT_X28_X29_OFFSET                                       \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X28_X29_OFFSET)
#define ARCH_TASK_CONTEXT_X30_SP_OFFSET                                        \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_X30_SP_OFFSET)
#define ARCH_TASK_CONTEXT_DAIF_NZCV_OFFSET                                     \
	(TASK_CONTEXT_OF_CALLEE_CONTEXT_OFFSET + CALLEE_CONTEXT_OF_DAIF_NZCV_OFFSET)
#ifdef CONFIG_FPU_ENABLE
#define ARCH_TASK_FP_CONTEXT_OFFSET TASK_CONTEXT_OF_FP_CONTEXT_OFFSET
#endif
#endif
