#ifndef __ARM64_GEN_OFFSET_H__
#define __ARM64_GEN_OFFSET_H__

/*
 * This header is used to generate offset symbols for use in
 * assembly code.
 */
#define offsetof(type, member) __builtin_offsetof(type, member)

#define GEN_ABS_SYM_BEGIN(name)                                                \
	extern void name(void);                                                    \
	void name(void) {

#define GEN_ABS_SYM_END }

#define GEN_ABSOLUTE_SYM(name, value)                                          \
	__asm__(".globl\t" #name "\n\t.equ\t" #name ",%c0"                         \
			"\n\t.type\t" #name ",@object"                                     \
			:                                                                  \
			: "n"(value))

#define GEN_NAMED_OFFSET_SYM(S, M, N)                                          \
	GEN_ABSOLUTE_SYM(N##_OFFSET, offsetof(S, M))

#define GEN_NAMED_SIZE_SYM(S, N) GEN_ABSOLUTE_SYM(N##_SIZE, sizeof(S))

#endif
