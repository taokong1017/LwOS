#ifndef __ARM64_FP_CONTEXT_H__
#define __ARM64_FP_CONTEXT_H__

#include <types.h>

struct arch_fp_context {
	__int128 q0,  q1,  q2,  q3,  q4,  q5,  q6,  q7;
	__int128 q8,  q9,  q10, q11, q12, q13, q14, q15;
	__int128 q16, q17, q18, q19, q20, q21, q22, q23;
	__int128 q24, q25, q26, q27, q28, q29, q30, q31;
	uint32_t fpsr, fpcr;
};

#endif