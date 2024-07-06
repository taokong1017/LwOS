#include <types.h>

#define stringify(x...) #x

#define read_sysreg(reg)                                                       \
	({                                                                         \
		uint64_t reg_val;                                                      \
		__asm__ volatile("mrs %0, " stringify(reg) : "=r"(reg_val)::"memory"); \
		reg_val;                                                               \
	})

#define write_sysreg(val, reg)                                                 \
	({ __asm__ volatile("msr " stringify(reg) ", %0" ::"r"(val) : "memory"); })

#define zero_sysreg(reg)                                                       \
	({ __asm__ volatile("msr " stringify(reg) ", xzr" ::: "memory"); })

#define define_reg_operation(reg)                                              \
	uint64_t read_##reg(void) { return read_sysreg(reg); }                     \
	void write_##reg(uint64_t val) { write_sysreg(val, reg); }                 \
	void zero_##reg(void) { zero_sysreg(reg); }

#define define_el123_reg_operation(reg)                                        \
	define_reg_operation(reg##_el1);                                           \
	define_reg_operation(reg##_el2);                                           \
	define_reg_operation(reg##_el3)

#define define_reg_read(reg)                                                   \
	uint64_t read_##reg(void) { return read_sysreg(reg); }

/* define special register operations */
define_reg_read(cntpct_el0);
define_reg_read(mpidr_el1);
define_reg_operation(cntfrq_el0);
define_reg_operation(cntp_ctl_el0);
define_reg_operation(cntp_cval_el0);
define_reg_operation(cntp_tval_el0);
define_reg_operation(csselr_el1);
define_reg_operation(hcr_el2);
define_reg_operation(tpidrro_el0);
define_reg_operation(sp_el0);
define_el123_reg_operation(actlr);
define_el123_reg_operation(elr);
define_el123_reg_operation(esr);
define_el123_reg_operation(far);
define_el123_reg_operation(mair);
define_el123_reg_operation(sctlr);
define_el123_reg_operation(spsr);
define_el123_reg_operation(tcr);
define_el123_reg_operation(ttbr0);
