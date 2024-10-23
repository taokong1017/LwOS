#ifndef __ARM64_SYS_REGISTERS_H__
#define __ARM64_SYS_REGISTERS_H__

/* MAIR_ELx memory attributes */
#define MAIR_ATTR_DEVICE_nGnRnE (0x00UL)
#define MAIR_ATTR_DEVICE_nGnRE (0x04UL)
#define MAIR_ATTR_DEVICE_GRE (0x0cUL)
#define MAIR_ATTR_NORMAL_NC (0x44UL)
#define MAIR_ATTR_NORMAL (0xffUL)
#define MAIR_ATTR_NORMAL_WT (0xbbUL)

/* Position the attr at the correct index */
#define MAIR_ATTRIDX(attr, idx) ((attr) << ((idx)*8))

/* Memory types available */
#define MT_DEVICE_nGnRnE 0U
#define MT_DEVICE_nGnRE 1U
#define MT_DEVICE_GRE 2U
#define MT_NORMAL_NC 3U
#define MT_NORMAL 4U
#define MT_NORMAL_WT 5U

#define MAIR_EL1_SET                                                           \
	(MAIR_ATTRIDX(MAIR_ATTR_DEVICE_nGnRnE, MT_DEVICE_nGnRnE) |                 \
	 MAIR_ATTRIDX(MAIR_ATTR_DEVICE_nGnRE, MT_DEVICE_nGnRE) |                   \
	 MAIR_ATTRIDX(MAIR_ATTR_DEVICE_GRE, MT_DEVICE_GRE) |                       \
	 MAIR_ATTRIDX(MAIR_ATTR_NORMAL_NC, MT_NORMAL_NC) |                         \
	 MAIR_ATTRIDX(MAIR_ATTR_NORMAL, MT_NORMAL) |                               \
	 MAIR_ATTRIDX(MAIR_ATTR_NORMAL_WT, MT_NORMAL_WT))

#define INIT_SCTLR_EL1_MMU_OFF                                                 \
	(ENDIAN_SET_EL1 | SCTLR_EL1_LSMAOE | SCTLR_EL1_nTLSMD | SCTLR_EL1_EIS |    \
	 SCTLR_EL1_TSCXT | SCTLR_EL1_EOS)

/* Common SCTLR_ELx flags. */
#define SCTLR_ELx_ENTP2 (BIT(60))
#define SCTLR_ELx_DSSBS (BIT(44))
#define SCTLR_ELx_ATA (BIT(43))
#define SCTLR_ELx_ITFSB (BIT(37))
#define SCTLR_ELx_ENIA (BIT(31))
#define SCTLR_ELx_ENIB (BIT(30))
#define SCTLR_ELx_LSMAOE (BIT(29))
#define SCTLR_ELx_nTLSMD (BIT(28))
#define SCTLR_ELx_ENDA (BIT(27))
#define SCTLR_ELx_EE (BIT(25))
#define SCTLR_ELx_EIS (BIT(22))
#define SCTLR_ELx_IESB (BIT(21))
#define SCTLR_ELx_TSCXT (BIT(20))
#define SCTLR_ELx_WXN (BIT(19))
#define SCTLR_ELx_ENDB (BIT(13))
#define SCTLR_ELx_I (BIT(12))
#define SCTLR_ELx_EOS (BIT(11))
#define SCTLR_ELx_SA (BIT(3))
#define SCTLR_ELx_C (BIT(2))
#define SCTLR_ELx_A (BIT(1))
#define SCTLR_ELx_M (BIT(0))

#define CTLR_EL1_MMU_ON                                                        \
	(SCTLR_ELx_M | SCTLR_ELx_A | SCTLR_ELx_C | SCTLR_ELx_SA | SCTLR_ELx_I)

#endif
