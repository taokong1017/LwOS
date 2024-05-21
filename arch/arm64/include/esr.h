#ifndef __ARM64_ESR_H__
#define __ARM64_ESR_H__

#define ESR_ELx_EC_UNKNOWN (0x00)
#define ESR_ELx_EC_WFx (0x01)
/* Unallocated EC: 0x02 */
#define ESR_ELx_EC_CP15_32 (0x03)
#define ESR_ELx_EC_CP15_64 (0x04)
#define ESR_ELx_EC_CP14_MR (0x05)
#define ESR_ELx_EC_CP14_LS (0x06)
#define ESR_ELx_EC_FP_ASIMD (0x07)
#define ESR_ELx_EC_CP10_ID (0x08) /* EL2 only */
#define ESR_ELx_EC_PAC (0x09)	  /* EL2 and above */
/* Unallocated EC: 0x0A - 0x0B */
#define ESR_ELx_EC_CP14_64 (0x0C)
#define ESR_ELx_EC_BTI (0x0D)
#define ESR_ELx_EC_ILL (0x0E)
/* Unallocated EC: 0x0F - 0x10 */
#define ESR_ELx_EC_SVC32 (0x11)
#define ESR_ELx_EC_HVC32 (0x12) /* EL2 only */
#define ESR_ELx_EC_SMC32 (0x13) /* EL2 and above */
/* Unallocated EC: 0x14 */
#define ESR_ELx_EC_SVC64 (0x15)
#define ESR_ELx_EC_HVC64 (0x16) /* EL2 and above */
#define ESR_ELx_EC_SMC64 (0x17) /* EL2 and above */
#define ESR_ELx_EC_SYS64 (0x18)
#define ESR_ELx_EC_SVE (0x19)
#define ESR_ELx_EC_ERET (0x1a) /* EL2 only */
/* Unallocated EC: 0x1B */
#define ESR_ELx_EC_FPAC (0x1C) /* EL1 and above */
#define ESR_ELx_EC_SME (0x1D)
/* Unallocated EC: 0x1E */
#define ESR_ELx_EC_IMP_DEF (0x1f) /* EL3 only */
#define ESR_ELx_EC_IABT_LOW (0x20)
#define ESR_ELx_EC_IABT_CUR (0x21)
#define ESR_ELx_EC_PC_ALIGN (0x22)
/* Unallocated EC: 0x23 */
#define ESR_ELx_EC_DABT_LOW (0x24)
#define ESR_ELx_EC_DABT_CUR (0x25)
#define ESR_ELx_EC_SP_ALIGN (0x26)
#define ESR_ELx_EC_MOPS (0x27)
#define ESR_ELx_EC_FP_EXC32 (0x28)
/* Unallocated EC: 0x29 - 0x2B */
#define ESR_ELx_EC_FP_EXC64 (0x2C)
/* Unallocated EC: 0x2D - 0x2E */
#define ESR_ELx_EC_SERROR (0x2F)
#define ESR_ELx_EC_BREAKPT_LOW (0x30)
#define ESR_ELx_EC_BREAKPT_CUR (0x31)
#define ESR_ELx_EC_SOFTSTP_LOW (0x32)
#define ESR_ELx_EC_SOFTSTP_CUR (0x33)
#define ESR_ELx_EC_WATCHPT_LOW (0x34)
#define ESR_ELx_EC_WATCHPT_CUR (0x35)
/* Unallocated EC: 0x36 - 0x37 */
#define ESR_ELx_EC_BKPT32 (0x38)
/* Unallocated EC: 0x39 */
#define ESR_ELx_EC_VECTOR32 (0x3A) /* EL2 only */
/* Unallocated EC: 0x3B */
#define ESR_ELx_EC_BRK64 (0x3C)
/* Unallocated EC: 0x3D - 0x3F */
#define ESR_ELx_EC_MAX (0x3F)

#define ESR_ELx_EC_SHIFT (26)
#define ESR_ELx_EC_WIDTH (6)
#define ESR_ELx_EC_MASK (0x3FUL << ESR_ELx_EC_SHIFT)
#define ESR_ELx_EC(esr) (((esr)&ESR_ELx_EC_MASK) >> ESR_ELx_EC_SHIFT)

#endif
