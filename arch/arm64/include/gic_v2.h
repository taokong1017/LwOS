#ifndef __ARM64_GIC_V2_H__
#define __ARM64_GIC_V2_H__

/* BIT(0) reserved for IRQ_ZERO_LATENCY */
#define IRQ_TYPE_LEVEL (1UL << 1)
#define IRQ_TYPE_EDGE (1UL << 2)

/* GIC Register Interface Base Addresses */
#define GIC_BASE 0x8000000
#define GIC_SIZE 0x20000
#define GIC_DIST_BASE (GIC_BASE)
#define GIC_CPU_BASE (GIC_BASE + 0x10000)

/* 0x000  Distributor Control Register */
#define GICD_CTLR (GIC_DIST_BASE + 0x0)

/* 0x004  Interrupt Controller Type Register */
#define GICD_TYPER (GIC_DIST_BASE + 0x4)

/* 0x008  Distributor Implementer Identification Register */
#define GICD_IIDR (GIC_DIST_BASE + 0x8)

/* 0x080  Interrupt Group Registers */
#define GICD_IGROUPRn (GIC_DIST_BASE + 0x80)

/* 0x100  Interrupt Set-Enable Registers */
#define GICD_ISENABLERn (GIC_DIST_BASE + 0x100)

/* 0x180  Interrupt Clear-Enable Registers */
#define GICD_ICENABLERn (GIC_DIST_BASE + 0x180)

/* 0x200  Interrupt Set-Pending Registers */
#define GICD_ISPENDRn (GIC_DIST_BASE + 0x200)

/* 0x280  Interrupt Clear-Pending Registers */
#define GICD_ICPENDRn (GIC_DIST_BASE + 0x280)

/* 0x300  Interrupt Set-Active Registers */
#define GICD_ISACTIVERn (GIC_DIST_BASE + 0x300)

/* 0x380  Interrupt Clear-Active Registers */
#define GICD_ICACTIVERn (GIC_DIST_BASE + 0x380)

/* 0x400  Interrupt Priority Registers */
#define GICD_IPRIORITYRn (GIC_DIST_BASE + 0x400)

/* 0x800  Interrupt Processor Targets Registers */
#define GICD_ITARGETSRn (GIC_DIST_BASE + 0x800)

/* 0xC00  Interrupt Configuration Registers */
#define GICD_ICFGRn (GIC_DIST_BASE + 0xc00)

/* 0xF00  Software Generated Interrupt Register */
#define GICD_SGIR (GIC_DIST_BASE + 0xf00)

/* 0x0000  CPU Interface Control Register */
#define GICC_CTLR (GIC_CPU_BASE + 0x0)

/* 0x0004  Interrupt Priority Mask Register */
#define GICC_PMR (GIC_CPU_BASE + 0x4)

/* 0x0008  Binary Point Register */
#define GICC_BPR (GIC_CPU_BASE + 0x8)

/* 0x000C  Interrupt Acknowledge Register */
#define GICC_IAR (GIC_CPU_BASE + 0xc)

/* 0x0010  End of Interrupt Register */
#define GICC_EOIR (GIC_CPU_BASE + 0x10)

/* 0x00fc  CPU Interface Identification Register */
#define GICC_IIDR (GIC_CPU_BASE + 0xfc)
#define GIC_V2_MASK 0x02043B

/* GICC_CTLR */
#define GICC_CTLR_ENABLEGRP0 (1UL)
#define GICC_CTLR_ENABLEGRP1 (1UL << 1)
#define GICC_CTLR_ENABLE_MASK (GICC_CTLR_ENABLEGRP0 | GICC_CTLR_ENABLEGRP1)

#define GICC_CTLR_FIQBYPDISGRP0 (1UL << 5)
#define GICC_CTLR_IRQBYPDISGRP0 (1UL << 6)
#define GICC_CTLR_FIQBYPDISGRP1 (1UL << 7)
#define GICC_CTLR_IRQBYPDISGRP1 (1UL << 8)
#define GICC_CTLR_BYPASS_MASK                                                  \
	(GICC_CTLR_FIQBYPDISGRP0 | GICC_CTLR_IRQBYPDISGRP1 |                       \
	 GICC_CTLR_FIQBYPDISGRP1 | GICC_CTLR_IRQBYPDISGRP1)

/* GICD_SGIR */
#define GICD_SGIR_TGTFILT(x) ((x) << 24)
#define GICD_SGIR_TGTFILT_CPULIST GICD_SGIR_TGTFILT(0b00)
#define GICD_SGIR_TGTFILT_ALLBUTREQ GICD_SGIR_TGTFILT(0b01)
#define GICD_SGIR_TGTFILT_REQONLY GICD_SGIR_TGTFILT(0b10)

#define GICD_SGIR_CPULIST(x) ((x) << 16)
#define GICD_SGIR_CPULIST_CPU(n) GICD_SGIR_CPULIST(1UL << n)
#define GICD_SGIR_CPULIST_MASK 0xff

#define GICD_SGIR_NSATT (1UL << 15)
#define GICD_SGIR_SGIINTID(x) (x)

/* GICD_ICFGR */
#define GICD_ICFGR_MASK ((1UL << 2) - 1)
#define GICD_ICFGR_TYPE (1UL << 1)

/* GICD_TYPER.ITLinesNumber 0:4 */
#define GICD_TYPER_ITLINESNUM_MASK 0x1f

/* GICD_TYPER.IDbits */
#define GICD_TYPER_IDBITS(typer) ((((typer) >> 19) & 0x1f) + 1)

/*
 * Common Helper Constants
 */
#define GIC_SGI_INT_BASE 0
#define GIC_PPI_INT_BASE 16
#define GIC_IS_SGI(intid)                                                      \
	(((intid) >= GIC_SGI_INT_BASE) && ((intid) < GIC_PPI_INT_BASE))

#define GIC_SPI_INT_BASE 32
#define GIC_SPI_MAX_INTID 1019
#define GIC_IS_SPI(intid)                                                      \
	(((intid) >= GIC_SPI_INT_BASE) && ((intid) <= GIC_SPI_MAX_INTID))

#define GIC_NUM_INTR_PER_REG 32
#define GIC_NUM_CFG_PER_REG 16
#define GIC_NUM_PRI_PER_REG 4

/* GIC idle priority : value '0xff' will allow all interrupts */
#define GIC_IDLE_PRIO 0xff
/* Priority levels 0:255 */
#define GIC_PRI_MASK 0xff

/*
 * '0xa0'is used to initialize each interrupt default priority.
 * This is an arbitrary value in current context.
 * Any value '0x80' to '0xff' will work for both NS and S state.
 * The values of individual interrupt and default has to be chosen
 * carefully if PMR and BPR based nesting and preemption has to be done.
 */
#define GIC_INT_DEF_PRI_X4 0xa0a0a0a0

/* GIC special interrupt id */
#define GIC_INTID_SPURIOUS 1023

#define GIC_NUM_CPU_IF CONFIG_CPUS_MAX_NUM

#ifndef __ASSEMBLY__

#include <types.h>

/**
 * @brief Enable interrupt
 *
 * @param irq interrupt ID
 */
void arm_gic_irq_enable(uint32_t irq);

/**
 * @brief Disable interrupt
 *
 * @param irq interrupt ID
 */
void arm_gic_irq_disable(uint32_t irq);

/**
 * @brief Check if an interrupt is enabled
 *
 * @param irq interrupt ID
 * @return Returns true if interrupt is enabled, false otherwise
 */
bool arm_gic_irq_is_enabled(uint32_t irq);

/**
 * @brief Check if an interrupt is pending
 *
 * @param irq interrupt ID
 * @return Returns true if interrupt is pending, false otherwise
 */
bool arm_gic_irq_is_pending(uint32_t irq);

/**
 * @brief Clear the pending irq
 *
 * @param irq interrupt ID
 */
void arm_gic_irq_clear_pending(uint32_t irq);

/**
 * @brief Set interrupt priority
 *
 * @param irq interrupt ID
 * @param prio interrupt priority
 * @param flags interrupt flags
 */
void arm_gic_irq_set_priority(uint32_t irq, uint32_t prio, uint32_t flags);

/**
 * @brief Get active interrupt ID
 *
 * @return Returns the ID of an active interrupt
 */
uint32_t arm_gic_get_active(void);

/**
 * @brief Signal end-of-interrupt
 *
 * @param irq interrupt ID
 */
void arm_gic_eoi(uint32_t irq);

/**
 * @brief raise SGI to target cores
 *
 * @param sgi_id      SGI ID 0 to 15
 * @param affinity  target affinity in mpidr form.
 *                    Aff level 1 2 3 will be extracted by api.
 * @param cpu_mask bitmask of target cores
 */
void gic_raise_sgi(uint32_t sgi_id, uint64_t affinity, uint16_t cpu_mask);

/**
 * @brief Initialize the GIC (Generic Interrupt Controller)
 */
void arm_gic_init(bool primary);

#endif
#endif
