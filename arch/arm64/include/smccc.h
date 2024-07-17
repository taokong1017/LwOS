#ifndef __ARM64_SMCCC_H__
#define __ARM64_SMCCC_H__

#include <types.h>

struct arch_smccc {
	uint64_t a0;
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
	uint64_t a4;
	uint64_t a5;
	uint64_t a6;
	uint64_t a7;
};

/*
 * @brief Make HVC calls
 *
 * @param a0 function identifier
 * @param a1-a7 parameters registers
 * @param res results
 */
void arch_smccc_hvc(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
					uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
					struct arch_smccc *res);

/*
 * @brief Make SMC calls
 *
 * @param a0 function identifier
 * @param a1-a7 parameters registers
 * @param res results
 */
void arch_smccc_smc(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
					uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7,
					struct arch_smccc *res);

#endif
