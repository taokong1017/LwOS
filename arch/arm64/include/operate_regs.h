#ifndef __ARM64_OPERATE_REGS_H__
#define __ARM64_OPERATE_REGS_H__

#include <types.h>

#define declare_reg_read(reg) uint64_t read_##reg(void);

#define declare_reg_operation(reg)                                             \
	uint64_t read_##reg(void);                                                 \
	void write_##reg(uint64_t val);                                            \
	void zero_##reg(void);

#define declare_el123_reg_operation(reg)                                       \
	declare_reg_operation(reg##_el1);                                          \
	declare_reg_operation(reg##_el2);                                          \
	declare_reg_operation(reg##_el3)

/* define special register operations */
declare_reg_read(cntvct_el0);
declare_reg_read(mpidr_el1);
declare_reg_operation(cntfrq_el0);
declare_reg_operation(cntv_ctl_el0);
declare_reg_operation(cntv_cval_el0);
declare_reg_operation(tpidrro_el0);
declare_reg_operation(sp_el0);
declare_el123_reg_operation(elr);
declare_el123_reg_operation(esr);
declare_el123_reg_operation(far);
declare_el123_reg_operation(mair);
declare_el123_reg_operation(sctlr);
declare_el123_reg_operation(spsr);
declare_el123_reg_operation(tcr);
declare_el123_reg_operation(ttbr0);

#endif
