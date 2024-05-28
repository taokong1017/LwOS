#include <log.h>
#include <gic_v2.h>
#include <compiler.h>
#include <menuconfig.h>

#define GIC_V2_TAG "GICv2"

static inline uint8_t sys_read8(uint32_t addr) {
	uint8_t val;

	__asm__ volatile("ldrb %w0, [%1]" : "=r"(val) : "r"(addr));
	dmb(sy);

	return val;
}

static inline void sys_write8(uint8_t data, uint32_t addr) {
	dmb(sy);
	__asm__ volatile("strb %w0, [%1]" : : "r"(data), "r"(addr));
}

static inline void sys_write32(uint32_t data, uint32_t addr) {
	dmb(sy);
	__asm__ volatile("str %w0, [%1]" : : "r"(data), "r"(addr));
}

static inline uint32_t sys_read32(uint32_t addr) {
	uint32_t val;

	__asm__ volatile("ldr %w0, [%1]" : "=r"(val) : "r"(addr));
	dmb(sy);

	return val;
}

void arm_gic_irq_enable(uint32_t irq) {
	int32_t group, offset;

	group = irq / 32;
	offset = irq % 32;

	sys_write32((1 << offset), (GICD_ISENABLERn + group * 4));
}

void arm_gic_irq_disable(uint32_t irq) {
	int32_t group, offset;

	group = irq / 32;
	offset = irq % 32;

	sys_write32((1 << offset), (GICD_ICENABLERn + group * 4));
}

bool arm_gic_irq_is_enabled(uint32_t irq) {
	int32_t group, offset;
	uint32_t enabler;

	group = irq / 32;
	offset = irq % 32;

	enabler = sys_read32(GICD_ISENABLERn + group * 4);

	return (enabler & (1 << offset)) != 0;
}

bool arm_gic_irq_is_pending(uint32_t irq) {
	int32_t group, offset;
	uint32_t enabler;

	group = irq / 32;
	offset = irq % 32;

	enabler = sys_read32(GICD_ISPENDRn + group * 4);

	return (enabler & (1 << offset)) != 0;
}

void arm_gic_irq_clear_pending(uint32_t irq) {
	int32_t group, offset;

	group = irq / 32;
	offset = irq % 32;

	sys_write32((1 << offset), (GICD_ICPENDRn + group * 4));
}

void arm_gic_irq_set_priority(uint32_t irq, uint32_t prio, uint32_t flags) {
	int32_t group, offset;
	uint32_t val;

	/* Set priority */
	sys_write8(prio & 0xff, GICD_IPRIORITYRn + irq);

	/* Set interrupt type */
	group = (irq / 16) * 4;
	offset = (irq % 16) * 2;

	val = sys_read32(GICD_ICFGRn + group);
	val &= ~(GICD_ICFGR_MASK << offset);
	if (flags & IRQ_TYPE_EDGE) {
		val |= (GICD_ICFGR_TYPE << offset);
	}

	sys_write32(val, GICD_ICFGRn + group);
}

uint32_t arm_gic_get_active() {
	uint32_t irq;

	irq = sys_read32(GICC_IAR) & 0x3ff;
	return irq;
}

void arm_gic_eoi(uint32_t irq) {
	/*
	 * Ensure the write to peripheral registers are *complete* before the write
	 * to GIC_EOIR.
	 *
	 * Note: The completion guarantee depends on various factors of system
	 * design and the barrier is the best core can do by which execution of
	 * further instructions waits till the barrier is alive.
	 */
	dsb(sy);

	/* set to inactive */
	sys_write32(irq, GICC_EOIR);
}

void gic_raise_sgi(uint32_t sgi_id, uint64_t affinity, uint16_t cpu_mask) {
	uint32_t sgi_val;

	sgi_val = GICD_SGIR_TGTFILT_CPULIST |
			  GICD_SGIR_CPULIST(cpu_mask & GICD_SGIR_CPULIST_MASK) | sgi_id;

	dsb(sy);
	sys_write32(sgi_val, GICD_SGIR);
	isb(sy);
}

static void gic_dist_init() {
	uint32_t gic_irqs, i;
	uint8_t cpu_mask = 0;
	uint32_t reg_val;

	gic_irqs = sys_read32(GICD_TYPER) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if (gic_irqs > 1020) {
		gic_irqs = 1020;
	}

	/*
	 * Disable the forwarding of pending interrupts
	 * from the Distributor to the CPU interfaces
	 */
	sys_write32(0, GICD_CTLR);

	/*
	 * Enable all global interrupts distributing to CPUs listed
	 * in dts with the count of arch_num_cpus().
	 */
	cpu_mask = (1 << CONFIG_CPUS_MAX_NUM) - 1;
	reg_val = cpu_mask | (cpu_mask << 8) | (cpu_mask << 16) | (cpu_mask << 24);
	for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 4) {
		sys_write32(reg_val, GICD_ITARGETSRn + i);
	}

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 16) {
		sys_write32(0, GICD_ICFGRn + i / 4);
	}

	/*  Set priority on all global interrupts.   */
	for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 4) {
		sys_write32(0, GICD_IPRIORITYRn + i);
	}

	/* Set all interrupts to group 0 */
	for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 32) {
		sys_write32(0, GICD_IGROUPRn + i / 8);
	}

	/*
	 * Disable all interrupts.  Leave the PPI and SGIs alone
	 * as these enables are banked registers.
	 */
	for (i = GIC_SPI_INT_BASE; i < gic_irqs; i += 32) {
		sys_write32(0xffffffff, GICD_ICENABLERn + i / 8);
	}

	/*
	 * Enable the forwarding of pending interrupts
	 * from the Distributor to the CPU interfaces
	 */
	sys_write32(1, GICD_CTLR);
}

static void gic_cpu_init() {
	int i;
	uint32_t val;

	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * PPI interrupts, ensure all SGI interrupts are enabled.
	 */
	sys_write32(0xffff0000, GICD_ICENABLERn);
	sys_write32(0x0000ffff, GICD_ISENABLERn);

	/*
	 * Set priority on PPI and SGI interrupts
	 */
	for (i = 0; i < 32; i += 4) {
		sys_write32(0xa0a0a0a0, GICD_IPRIORITYRn + i);
	}

	sys_write32(0xf0, GICC_PMR);

	/*
	 * Enable interrupts and signal them using the IRQ signal.
	 */
	val = sys_read32(GICC_CTLR);
	val |= GICC_CTLR_ENABLE_MASK;
	sys_write32(val, GICC_CTLR);
}

/**
 * @brief check gic archtecture version
 */
static bool gic_v2_check(void) {
	uint32_t id = sys_read32(GICC_IIDR);

	if (id == GIC_V2_MASK) {
		return true;
	}

	return false;
}

/**
 * @brief Initialize the GIC driver
 */
void arm_gic_init() {
	if (!gic_v2_check()) {
		log_err(GIC_V2_TAG, "GICv2 not available\n");
		return;
	}

	gic_dist_init();
	gic_cpu_init();
}
